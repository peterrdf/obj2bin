#pragma once

#include "_exporter.h"

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

		map<int64_t, bool> m_mapVertexFilter;
		map<int64_t, bool> m_mapFaceFilter;

	public: // Methods

		_cropping(const char* szInputFile, const char* szOutputFile);
		virtual ~_cropping();

		// _exporter
		virtual void execute() override;		

	private: // Methods

		void calculateBB();
		void run();

	public: // Properties

		bool isFaceFiltered(int64_t iFace) const;
	};
};

