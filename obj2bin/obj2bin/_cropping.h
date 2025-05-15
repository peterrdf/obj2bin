#pragma once

#include "_obj.h"

// ************************************************************************************************
namespace _obj2bin
{
	// ********************************************************************************************
	class _cropping : public _exporter
	{

	private: // Fields

		double m_dBBXMin;
		double m_dBBYMin;
		double m_dBBZMin;
		double m_dBBXMax;
		double m_dBBYMax;
		double m_dBBZMax;

		map<int64_t, bool> m_mapVertexFilter; /*0-based*/
		map<int64_t, bool> m_mapFaceFilter; /*0-based*/

		map<int64_t, vector<int64_t>> m_mapVertex2Faces; /*0-based*/
		set<int64_t> m_setFaceNeighborsAdded; /*0-based*/

	public: // Methods

		_cropping(const char* szInputFile, const char* szOutputFile);
		virtual ~_cropping();

		// _exporter
		virtual void execute() override;		

	private: // Methods

		void calculateBB();
		void run();
		void getFaceNeighbors(int64_t iFace/*0-based*/, vector<int64_t>& vecFaceNeighbors/*0-based*/);

	public: // Properties

		bool isFaceFiltered(int64_t iFace/*0-based*/) const;
	};
};

