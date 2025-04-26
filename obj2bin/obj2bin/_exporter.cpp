#include "pch.h"
#include "_exporter.h"

#include <atlstr.h>
#include "_string.h"

// ************************************************************************************************
namespace _obj2bin
{
	// ********************************************************************************************
	static const char default_material_name[] = "Default Material";
	static const char default_color_name[] = "Default Color";

	// ********************************************************************************************
	_exporter::_exporter(const char* szInputFile, const char* szOutputFile)
		: _log_client()
		, m_iModel(0)
		, m_strInputFile()
		, m_strOutputFile()
		, m_setMaterialLibraries()
		, m_vecMaterials()
		, m_mapMaterials()
		, m_iDefaultMaterialInstance(0)
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

		for (auto& itMaterial : m_mapMaterials) {
			delete itMaterial.second.first;
		}
	}

	/*virtual*/ void _exporter::execute()
	{
		//
		// OBJ
		//

		load();

		//
		// Materials
		//

		loadMaterials();

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

				if (!m_vecTextureUVs.empty()) {
					long iUVIndex = atol(vecFaceVertex[1].c_str()) - 1;
					m_vecBRepTextureUVs.push_back(m_vecTextureUVs[(iUVIndex * 2) + 0]);
					m_vecBRepTextureUVs.push_back(m_vecTextureUVs[(iUVIndex * 2) + 1]);
				}

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

		if (!m_vecBRepNormals.empty()) {
			SetDatatypeProperty(
				owlBRepInstance,
				GetPropertyByName(m_iModel, "textureCoordinates"),
				m_vecBRepTextureUVs.data(),
				m_vecBRepTextureUVs.size());
		}		

		SetObjectProperty(
			owlBRepInstance, 
			GetPropertyByName(m_iModel, 
				"material"), 
			getMaterialInstance());

		SaveModel(m_iModel, m_strOutputFile.c_str());
	}

	void _exporter::load()
	{
		auto pReader = new _file_reader();
		if (!pReader->open(m_strInputFile.c_str())) {
			THROW_ERROR(_err::_file);
		}

		char ch = pReader->getChar();

		if (ch == EOF) {
			THROW_ERROR(_err::_file);
		}

		string strLine;
		while (ch != EOF) {
			if ((ch == LF) || (ch == CR)) {
				processOBJLine(strLine);
				strLine = "";

				ch = pReader->getNextChar(false);
				continue;
			}

			strLine += ch;
			ch = pReader->getNextChar(false);
		}

		delete pReader;
	}

	void _exporter::processOBJLine(const string& strLine)
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

			m_vecMaterials.push_back(vecTokens[1]); //#todo: multiple materials
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
			string strWarning = "Not support element: '"; 
			strWarning += strLine.substr(0, 10);
			strWarning += "...'";
			getLog()->logWrite(enumLogEvent::warning, strWarning);
		}
	}

	void _exporter::loadMaterials()
	{
		fs::path pathInputFolder = m_strInputFile;
		pathInputFolder = pathInputFolder.parent_path();

		for (const auto& strMaterialLibarary : m_setMaterialLibraries) {
			fs::path pathMaterialLibarary = pathInputFolder;
			pathMaterialLibarary.append(strMaterialLibarary);

			if (!fs::exists(pathMaterialLibarary)) {
				continue;
			}

			auto pReader = new _file_reader();
			if (!pReader->open(pathMaterialLibarary.string().c_str())) {
				THROW_ERROR(_err::_file);
			}

			char ch = pReader->getChar();

			if (ch == EOF) {
				THROW_ERROR(_err::_file);
			}

			vector<string> vecTokens;
			string strMaterial;			

			string strLine;
			while (ch != EOF) {
				if ((ch == LF) || (ch == CR)) {
					if (strLine.find("#") == 0) {
						// Comment
						getLog()->logWrite(enumLogEvent::info, strLine);
					} else if (strLine.find("newmtl ") == 0) {
						// Material
						_string::split(strLine, " ", vecTokens, false);
						VERIFY_EXPRESSION(vecTokens.size() == 2);

						strMaterial = vecTokens[1];
					} else if (strLine.find("Ka ") == 0) {
						// Ambient color
						VERIFY_STLOBJ_IS_NOT_EMPTY(strMaterial);
						getLog()->logWrite(enumLogEvent::info, strLine);
					} else if (strLine.find("Kd ") == 0) {
						// Diffuse color
						VERIFY_STLOBJ_IS_NOT_EMPTY(strMaterial);
						getLog()->logWrite(enumLogEvent::info, strLine);
					} else if (strLine.find("Ks ") == 0) {
						// Specular color
						VERIFY_STLOBJ_IS_NOT_EMPTY(strMaterial);
						getLog()->logWrite(enumLogEvent::info, strLine);
					} else if (strLine.find("Ns ") == 0) {
						// Specular exponent weight
						VERIFY_STLOBJ_IS_NOT_EMPTY(strMaterial);
						getLog()->logWrite(enumLogEvent::info, strLine);
					} else if (strLine.find("d ") == 0) {
						// Transparency
						VERIFY_STLOBJ_IS_NOT_EMPTY(strMaterial);
						getLog()->logWrite(enumLogEvent::info, strLine);
					} else if (strLine.find("illum ") == 0) {
						// Illumination model
						VERIFY_STLOBJ_IS_NOT_EMPTY(strMaterial);
						getLog()->logWrite(enumLogEvent::info, strLine);
					} else if (strLine.find("map_Kd ") == 0) {
						// Texture
						VERIFY_STLOBJ_IS_NOT_EMPTY(strMaterial);
						getLog()->logWrite(enumLogEvent::info, strLine);

						_string::split(strLine, " ", vecTokens, false);
						VERIFY_EXPRESSION(vecTokens.size() == 2);
						
						auto pMaterial = new _material(0, 0, 0, 0, 1.f, (LPCWSTR)CA2W(vecTokens[1].c_str())); //#todo: materials without texture
						VERIFY_EXPRESSION(m_mapMaterials.find(strMaterial) == m_mapMaterials.end());
						m_mapMaterials[strMaterial] = { pMaterial, 0 };

						strMaterial = "";
					} else {
						string strWarning = "Not support element: '";
						strWarning += strLine.substr(0, 10);
						strWarning += "...'";
						getLog()->logWrite(enumLogEvent::warning, strWarning);
					}
					
					strLine = "";

					ch = pReader->getNextChar(false);
					continue;
				}

				strLine += ch;
				ch = pReader->getNextChar(false);
			}

			delete pReader;
		} // for (const auto& strMaterialLibarary : ...
	}

	void _exporter::createDefaultMaterial()
	{
		m_iDefaultMaterialInstance = CreateInstance(GetClassByName(m_iModel, "Material"));
		VERIFY_INSTANCE(m_iDefaultMaterialInstance);

		OwlInstance owlColorInstance = CreateInstance(GetClassByName(m_iModel, "Color"));
		VERIFY_INSTANCE(owlColorInstance);

		SetObjectProperty(
			m_iDefaultMaterialInstance,
			GetPropertyByName(m_iModel, "color"),
			&owlColorInstance,
			1);

		OwlInstance owlColorComponentInstance = createColorComponentInstance(default_color_name, 0., 0., 1.);
		VERIFY_INSTANCE(owlColorComponentInstance);

		SetObjectProperty(
			owlColorInstance,
			GetPropertyByName(m_iModel, "ambient"),
			&owlColorComponentInstance,
			1);

		double dTransparency = 0.25;
		SetDatatypeProperty(
			owlColorInstance,
			GetPropertyByName(m_iModel, "transparency"),
			&dTransparency,
			1);
	}

	OwlInstance _exporter::createColorComponentInstance(const string& strName, double dR, double dG, double dB)
	{
		OwlInstance owlColorComponentInstance = CreateInstance(GetClassByName(m_iModel, "ColorComponent"));
		VERIFY_INSTANCE(owlColorComponentInstance);

		SetDatatypeProperty(
			owlColorComponentInstance,
			GetPropertyByName(m_iModel, "R"),
			&dR,
			1);

		SetDatatypeProperty(
			owlColorComponentInstance,
			GetPropertyByName(m_iModel, "G"),
			&dG,
			1);

		SetDatatypeProperty(
			owlColorComponentInstance,
			GetPropertyByName(m_iModel, "B"),
			&dB,
			1);

		return owlColorComponentInstance;
	}

	OwlInstance _exporter::getDefaultMaterialInstance()
	{
		if (m_iDefaultMaterialInstance == 0) {
			createDefaultMaterial();
		}

		return m_iDefaultMaterialInstance;
	}

	OwlInstance _exporter::getMaterialInstance()
	{
		if (m_vecMaterials.empty()) {
			return getDefaultMaterialInstance();
		}

		auto itMaterial = m_mapMaterials.find(m_vecMaterials.back());
		if (itMaterial == m_mapMaterials.end()) {
			string stError = "Unknown material: '";
			stError += m_vecMaterials.back();
			stError += "'";
			getLog()->logWrite(enumLogEvent::error, stError);

			return getDefaultMaterialInstance();
		}

		if (!itMaterial->second.first->hasTexture()) {
			string stError = "Not supported material: '";
			stError += m_vecMaterials.back();
			stError += "'";
			getLog()->logWrite(enumLogEvent::error, stError);

			return getDefaultMaterialInstance();
		}

		if (itMaterial->second.second == 0) {
			string strTexture = (LPCSTR)CW2A(itMaterial->second.first->texture().c_str());

			char** szTexture = new char* [1];
			szTexture[0] = new char[strlen(strTexture.c_str()) + 1];
			strcpy(szTexture[0], strTexture.c_str());

			OwlInstance owlTextureInstance = CreateInstance(GetClassByName(m_iModel, "Texture"));
			SetDatatypeProperty(owlTextureInstance, GetPropertyByName(m_iModel, "name"), (void**)szTexture, 1);

			itMaterial->second.second = CreateInstance(GetClassByName(m_iModel, "Material"));
			SetObjectProperty(itMaterial->second.second, GetPropertyByName(m_iModel, "textures"), owlTextureInstance);
		}

		return itMaterial->second.second;
	}
};