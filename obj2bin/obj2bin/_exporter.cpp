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
		, m_vecIndices()
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
		// Output
		//

		m_iModel = CreateModel();
		assert(m_iModel != 0);

		OwlClass iClass = GetClassByName(m_iModel, "BoundaryRepresentation");
		VERIFY_INSTANCE(iClass);

		OwlInstance iInstance = CreateInstance(iClass);
		VERIFY_INSTANCE(iInstance);

		SetDatatypeProperty(
			iInstance,
			GetPropertyByName(m_iModel, "indices"),
			m_vecIndices.data(),
			m_vecIndices.size());

		SetDatatypeProperty(
			iInstance,
			GetPropertyByName(m_iModel, "vertices"),
			m_vecVertices.data(),
			m_vecVertices.size());

		SetDatatypeProperty(
			iInstance,
			GetPropertyByName(m_iModel, "normalCoordinates"),
			m_vecNormals.data(),
			m_vecNormals.size());

		SetDatatypeProperty(
			iInstance,
			GetPropertyByName(m_iModel, "textureCoordinates"),
			m_vecTextureUVs.data(),
			m_vecTextureUVs.size());

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
			m_vecTextureUVs.push_back(atof(vecTokens[2].c_str()));
		} else if (strLine.find("vn ") == 0) {
			// Normals
			_string::split(strLine, " ", vecTokens, false);
			VERIFY_EXPRESSION(vecTokens.size() == 4);

			m_vecNormals.push_back(atof(vecTokens[1].c_str()));
			m_vecNormals.push_back(atof(vecTokens[2].c_str()));
			m_vecNormals.push_back(atof(vecTokens[3].c_str()));
		} else if (strLine.find("f ") == 0) {
			// Faces
			_string::split(strLine, " ", vecTokens, false);
			VERIFY_EXPRESSION(vecTokens.size() == 4);

			m_vecFaces.push_back(vecTokens[1].c_str());
			m_vecFaces.push_back(vecTokens[2].c_str());
			m_vecFaces.push_back(vecTokens[3].c_str());
		} else {
			assert(false);
		}
	}
};