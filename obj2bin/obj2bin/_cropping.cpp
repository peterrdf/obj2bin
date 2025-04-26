#include "pch.h"
#include "_cropping.h"

// ************************************************************************************************
namespace _obj2bin
{
	_cropping::_cropping(const char* szInputFile, const char* szOutputFile)
		: _exporter(szInputFile, szOutputFile)
		, m_dBBXMin(-1)
		, m_dBBYMin(-1)
		, m_dBBZMin(-1)
		, m_dBBXMax(1)
		, m_dBBYMax(1)
		, m_dBBZMax(1)
	{
	}

	/*virtual*/ _cropping::~_cropping()
	{
	}

	/*virtual*/ void _cropping::execute() /*override*/
	{
		load();

		run();
	}	

	void _cropping::calculateBB()
	{
		for (size_t iVertex = 0; iVertex < m_vecVertices.size() / 3; iVertex += 3) {
			m_dBBXMin = fmin(m_dBBXMin, m_vecVertices[(iVertex * 3) + 0]);
			m_dBBXMax = fmax(m_dBBXMax, m_vecVertices[(iVertex * 3) + 0]);
			m_dBBYMin = fmin(m_dBBYMin, m_vecVertices[(iVertex * 3) + 1]);
			m_dBBYMax = fmax(m_dBBYMax, m_vecVertices[(iVertex * 3) + 1]);
			m_dBBZMin = fmin(m_dBBZMin, m_vecVertices[(iVertex * 3) + 2]);
			m_dBBZMax = fmax(m_dBBZMax, m_vecVertices[(iVertex * 3) + 2]);
		}

		string strInfo = _string::format("\n*** Scale I *** => Xmin/max, Ymin/max, Zmin/max: %.16f, %.16f, %.16f, %.16f, %.16f, %.16f",
			m_dBBXMin, m_dBBYMin,
			m_dBBZMin, m_dBBXMax,
			m_dBBYMax, m_dBBZMax);
		getLog()->logWrite(enumLogEvent::info, strInfo);
	}

	void _cropping::run()
	{
		calculateBB();
	}
};
