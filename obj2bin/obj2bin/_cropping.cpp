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
		, m_mapVertexFilter()
		, m_mapFaceFilter()
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

		// Front
		{
			// Top-Right
			{
				double dXMin = (m_dBBXMin + m_dBBXMax) / 2.;
				double dXMax = m_dBBXMax;
				double dYMin = (m_dBBYMin + m_dBBYMax) / 2.;
				double dYMax = m_dBBYMax;
				double dZMin = m_dBBZMin;
				double dZMax = (m_dBBZMin + m_dBBZMax) / 2.;

				for (size_t iVertex = 0; iVertex < m_vecVertices.size() / 3; iVertex += 3) {
					double dX = m_vecVertices[(iVertex * 3) + 0];
					double dY = m_vecVertices[(iVertex * 3) + 1];
					double dZ = m_vecVertices[(iVertex * 3) + 2];

					bool bFilter = false;
					if ((dX < dXMin) || (dX > dXMin) ||
						(dY < dYMin) || (dY > dYMin) ||
						(dZ < dZMin) || (dX > dZMin)) {
						bFilter = true;
					}

					m_mapVertexFilter[iVertex + 1] = bFilter;
				}
			} // Top-Right
		}
		// Front

		///////////////////////////////////////////////////////////////////////

		for (size_t iFace = 0; iFace < m_vecFaces.size(); iFace++) {
			vector<string> vecTokens;
			_string::split(m_vecFaces[iFace], " ", vecTokens, false);
			VERIFY_EXPRESSION(vecTokens.size() == 4);

			for (size_t iFaceVertex = 1; iFaceVertex < vecTokens.size(); iFaceVertex++) {
				vector<string> vecFaceVertex;
				_string::split(vecTokens[iFaceVertex], "/", vecFaceVertex, false);
				VERIFY_EXPRESSION(vecFaceVertex.size() == 3);

				int64_t iVertexIndex = atol(vecFaceVertex[0].c_str());
				if (m_mapVertexFilter[iVertexIndex]) {
					m_mapFaceFilter[iFace + 1] = true;
					break;
				} else {
					m_mapFaceFilter[iFace + 1] = false;
				}				
			}
		} // for (size_t iFace = ...
	}
};
