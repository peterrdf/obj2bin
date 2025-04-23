#include "pch.h"
#include "_exporter.h"

#include "_string.h"

// ************************************************************************************************
namespace _obj2bin
{
	// ************************************************************************************************
	_exporter::_exporter(const char* szInputFile, const char* szOutputFile)
		: _log_client()
		, m_iModel(0)
		, m_strInputFile()
		, m_strOutputFile()
		, m_setMaterialLibraries()
		, m_setMaterials()
		, m_vecVertices()
		, m_vecNormals()
		, m_vecTextureUVs()
		, m_vecFaces()
		, m_vecBRepIndices()
		, m_vecBRepVertices()
		, m_vecBRepNormals()
		, m_vecBRepTextureUVs()
	{
		VERIFY_POINTER(szInputFile);
		VERIFY_POINTER(szOutputFile);

		m_strInputFile = szInputFile;
		m_strOutputFile = szOutputFile;
	}

	/*virtual*/ _exporter::~_exporter()
	{
		if (m_iModel != 0) {
			CloseModel(m_iModel);
		}
	}

	void _exporter::execute()
	{
		auto pReader = new _file_reader();
		if (!pReader->open(m_strInputFile.c_str())) {
			THROW_ERROR(_err::_file);
		}

		char ch = pReader->getChar();

		if (ch == EOF) {
			THROW_ERROR(_err::_file);
		}

		string stLine;
		while (ch != EOF) {
			if ((ch == LF) || (ch == CR)) {
				processLine(stLine);
				stLine = "";

				ch = pReader->getNextChar(false);
				continue;
			}

			stLine += ch;
			ch = pReader->getNextChar(false);
		}

		delete pReader;

		//
		// BRep
		//

		int64_t iIndex = 0;
		for (size_t iFace = 0; iFace < m_vecFaces.size(); iFace++) {
			vector<string> vecTokens;
			_string::split(m_vecFaces[iFace], " ", vecTokens, false);
			VERIFY_EXPRESSION(vecTokens.size() == 4);

			for (size_t iFaceVertex = 1; iFaceVertex < vecTokens.size(); iFaceVertex++) {
				vector<string> vecFaceVertex;
				_string::split(vecTokens[iFaceVertex], "/", vecFaceVertex, false);
				VERIFY_EXPRESSION(vecFaceVertex.size() == 3);			

				m_vecBRepIndices.push_back(iIndex++);

				long iVertexIndex = atol(vecFaceVertex[0].c_str()) - 1;
				m_vecBRepVertices.push_back(m_vecVertices[(iVertexIndex * 3) + 0]);
				m_vecBRepVertices.push_back(m_vecVertices[(iVertexIndex * 3) + 1]);
				m_vecBRepVertices.push_back(m_vecVertices[(iVertexIndex * 3) + 2]);

				long iUVIndex = atol(vecFaceVertex[1].c_str()) - 1;
				m_vecBRepTextureUVs.push_back(m_vecTextureUVs[(iUVIndex * 2) + 0]);
				m_vecBRepTextureUVs.push_back(m_vecTextureUVs[(iUVIndex * 2) + 1]);

				long iNormalIndex = atol(vecFaceVertex[2].c_str()) - 1;
				m_vecBRepNormals.push_back(m_vecNormals[(iNormalIndex * 3) + 0]);
				m_vecBRepNormals.push_back(m_vecNormals[(iNormalIndex * 3) + 1]);
				m_vecBRepNormals.push_back(m_vecNormals[(iNormalIndex * 3) + 2]);
			}

			m_vecBRepIndices.push_back(-1);
		} // for (size_t iFace = ...

		m_iModel = CreateModel();
		assert(m_iModel != 0);

		OwlInstance owlBRepInstance = CreateInstance(GetClassByName(m_iModel, "BoundaryRepresentation"));
		VERIFY_INSTANCE(owlBRepInstance);

		SetDatatypeProperty(
			owlBRepInstance,
			GetPropertyByName(m_iModel, "indices"),
			m_vecBRepIndices.data(),
			m_vecBRepIndices.size());

		SetDatatypeProperty(
			owlBRepInstance,
			GetPropertyByName(m_iModel, "vertices"),
			m_vecBRepVertices.data(),
			m_vecBRepVertices.size());

		SetDatatypeProperty(
			owlBRepInstance,
			GetPropertyByName(m_iModel, "normalCoordinates"),
			m_vecBRepNormals.data(),
			m_vecBRepNormals.size());

		SetDatatypeProperty(
			owlBRepInstance,
			GetPropertyByName(m_iModel, "textureCoordinates"),
			m_vecBRepTextureUVs.data(),
			m_vecBRepTextureUVs.size());

		VERIFY_EXPRESSION(m_setMaterials.size() == 1);

		OwlInstance owlMaterialInstance = CreateInstance(GetClassByName(m_iModel, "Material"));
		SetObjectProperty(owlBRepInstance, GetPropertyByName(m_iModel, "material"), owlMaterialInstance);

		OwlInstance owlTextureInstance = CreateInstance(GetClassByName(m_iModel, "Texture"));
		SetObjectProperty(owlMaterialInstance, GetPropertyByName(m_iModel, "textures"), owlTextureInstance);

		char** szMaterial = new char* [1];
		szMaterial[0] = new char[strlen("material_0.png") + 1];
		strcpy(szMaterial[0], "material_0.png");

		SetDatatypeProperty(owlTextureInstance, GetPropertyByName(m_iModel, "name"), (void**)szMaterial, 1);

		SaveModel(m_iModel, m_strOutputFile.c_str());
	}

	void _exporter::processLine(const string& strLine)
	{
		vector<string> vecTokens;

		if (strLine.find("#") == 0) {
			// Comment
			getLog()->logWrite(enumLogEvent::info, strLine);
		} else if (strLine.find("mtllib ") == 0) {
			// Material Library
			_string::split(strLine, " ", vecTokens, false);
			VERIFY_EXPRESSION(vecTokens.size() == 2);

			m_setMaterialLibraries.insert(vecTokens[1]);
		} else if (strLine.find("usemtl ") == 0) {
			// Current Material
			_string::split(strLine, " ", vecTokens, false);
			VERIFY_EXPRESSION(vecTokens.size() == 2);

			m_setMaterials.push_back(vecTokens[1]);
		} else if (strLine.find("v ") == 0) {
			// Vertex
			_string::split(strLine, " ", vecTokens, false);
			VERIFY_EXPRESSION(vecTokens.size() == 4);

			m_vecVertices.push_back(atof(vecTokens[1].c_str()));
			m_vecVertices.push_back(atof(vecTokens[2].c_str()));
			m_vecVertices.push_back(atof(vecTokens[3].c_str()));
		} else if (strLine.find("vt ") == 0) {
			// Texture UV
			_string::split(strLine, " ", vecTokens, false);
			VERIFY_EXPRESSION(vecTokens.size() == 3);

			m_vecTextureUVs.push_back(atof(vecTokens[1].c_str()));
			m_vecTextureUVs.push_back(-atof(vecTokens[2].c_str()));
		} else if (strLine.find("vn ") == 0) {
			// Normals
			_string::split(strLine, " ", vecTokens, false);
			VERIFY_EXPRESSION(vecTokens.size() == 4);

			m_vecNormals.push_back(atof(vecTokens[1].c_str()));
			m_vecNormals.push_back(atof(vecTokens[2].c_str()));
			m_vecNormals.push_back(atof(vecTokens[3].c_str()));
		} else if (strLine.find("f ") == 0) {
			// Faces
			m_vecFaces.push_back(strLine);
		} else {
			assert(false);
		}
	}
};