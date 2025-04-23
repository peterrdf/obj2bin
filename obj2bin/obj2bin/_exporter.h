#pragma once

#ifdef _WINDOWS
#include "engine.h"
#else
#include "../../include/engine.h"
#endif

#include "_log.h"
#include "_io.h"
#include "_errors.h"
#include "_reader.h"

#include <set>
#include <map>
using namespace std;

// ************************************************************************************************
namespace _obj2bin
{
	// ********************************************************************************************
	class _exporter : public _log_client
	{

	private: // Members

		OwlModel m_iModel;
		string m_strInputFile;
		string m_strOutputFile;

		// Materials
		set<string> m_setMaterialLibraries;
		set<string> m_setMaterials;
		OwlInstance m_iDefaultMaterialInstance;

		// Input
		vector<double> m_vecVertices;
		vector<double> m_vecNormals;
		vector<double> m_vecTextureUVs;
		vector<string> m_vecFaces;

		// BRep
		vector<int64_t> m_vecBRepIndices;
		vector<double> m_vecBRepVertices;
		vector<double> m_vecBRepNormals;
		vector<double> m_vecBRepTextureUVs;
		

	public: // Methods

		_exporter(const char* szInputFile, const char* szOutputFile);
		virtual ~_exporter();

		void execute();

	private: // Methods

		void processOBJLine(const string& strLine);

		void loadMaterials();
		void createDefaultMaterial();
		OwlInstance getDefaultMaterialInstance();
	};
};

