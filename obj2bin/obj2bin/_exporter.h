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

		// Header
		int m_iVerticesCount;
		int m_iFacesCount;

		// Geometry
		vector<double> m_vecVertices;
		vector<int64_t> m_vecIndices;

	public: // Methods

		_exporter(const char* szInputFile, const char* szOutputFile);
		virtual ~_exporter();

		void execute();

	private: // Methods

		bool readHeader(int& iDataOffset);
	};
};

