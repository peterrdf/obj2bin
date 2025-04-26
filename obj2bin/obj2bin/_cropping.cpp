#include "pch.h"
#include "_cropping.h"

#include "_3DUtils.h"

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
			m_dBBXMin, m_dBBXMax,
			m_dBBYMin, m_dBBYMax,
			m_dBBXMin, m_dBBZMax);
		getLog()->logWrite(enumLogEvent::info, strInfo);
	}

	void _cropping::run()
	{
		calculateBB();

		// Front
		{
			// Left
			{
				double dXMin = m_dBBXMin;
				double dXMax = (m_dBBXMin + m_dBBXMax) / 2.;

				for (size_t iFace = 0; iFace < m_vecFaces.size(); iFace++) {
					vector<string> vecTokens;
					_string::split(m_vecFaces[iFace], " ", vecTokens, false);
					VERIFY_EXPRESSION(vecTokens.size() == 4);

					// Vertex 1
					vector<string> vecFaceVertex1;
					_string::split(vecTokens[1], "/", vecFaceVertex1, false);
					VERIFY_EXPRESSION(vecFaceVertex1.size() == 3);

					long iVertex1 = atol(vecFaceVertex1[0].c_str()) - 1;
					double dX1 = m_vecVertices[(iVertex1 * 3) + 0];
					double dY1 = m_vecVertices[(iVertex1 * 3) + 1];
					double dZ1 = m_vecVertices[(iVertex1 * 3) + 2];

					// Vertex 2
					vector<string> vecFaceVertex2;
					_string::split(vecTokens[2], "/", vecFaceVertex2, false);
					VERIFY_EXPRESSION(vecFaceVertex2.size() == 3);

					long iVertex2 = atol(vecFaceVertex2[0].c_str()) - 1;
					double dX2 = m_vecVertices[(iVertex2 * 3) + 0];
					double dY2 = m_vecVertices[(iVertex2 * 3) + 1];
					double dZ2 = m_vecVertices[(iVertex2 * 3) + 2];

					// Vertex 3
					vector<string> vecFaceVertex3;
					_string::split(vecTokens[3], "/", vecFaceVertex3, false);
					VERIFY_EXPRESSION(vecFaceVertex3.size() == 3);

					long iVertex3 = atol(vecFaceVertex3[0].c_str()) - 1;
					double dX3 = m_vecVertices[(iVertex3 * 3) + 0];
					double dY3 = m_vecVertices[(iVertex3 * 3) + 1];
					double dZ3 = m_vecVertices[(iVertex3 * 3) + 2];

					bool bFilter = false;
					if ((dX1 >= dXMin) && (dX1 <= dXMax) &&
						(dX2 >= dXMin) && (dX2 <= dXMax) &&
						(dX3 >= dXMin) && (dX3 <= dXMax)) {
						_vector3f v1(dX1, dY1, dZ1);
						_vector3f v2(dX2, dY2, dZ2);
						_vector3f normal = v2.cross(v1);
						if (normal.getX() > 0) {
							bFilter = true;
						}
					}

					m_mapFaceFilter[iFace + 1] = bFilter;
				} // for (size_t iFace = ...
			} // Left

			// Right
			{
				double dXMin = (m_dBBXMin + m_dBBXMax) / 2.;
				double dXMax = m_dBBXMax;

				for (size_t iFace = 0; iFace < m_vecFaces.size(); iFace++) {
					vector<string> vecTokens;
					_string::split(m_vecFaces[iFace], " ", vecTokens, false);
					VERIFY_EXPRESSION(vecTokens.size() == 4);

					// Vertex 1
					vector<string> vecFaceVertex1;
					_string::split(vecTokens[1], "/", vecFaceVertex1, false);
					VERIFY_EXPRESSION(vecFaceVertex1.size() == 3);

					long iVertex1 = atol(vecFaceVertex1[0].c_str()) - 1;
					double dX1 = m_vecVertices[(iVertex1 * 3) + 0];
					double dY1 = m_vecVertices[(iVertex1 * 3) + 1];
					double dZ1 = m_vecVertices[(iVertex1 * 3) + 2];

					// Vertex 2
					vector<string> vecFaceVertex2;
					_string::split(vecTokens[2], "/", vecFaceVertex2, false);
					VERIFY_EXPRESSION(vecFaceVertex2.size() == 3);

					long iVertex2 = atol(vecFaceVertex2[0].c_str()) - 1;
					double dX2 = m_vecVertices[(iVertex2 * 3) + 0];
					double dY2 = m_vecVertices[(iVertex2 * 3) + 1];
					double dZ2 = m_vecVertices[(iVertex2 * 3) + 2];

					// Vertex 3
					vector<string> vecFaceVertex3;
					_string::split(vecTokens[3], "/", vecFaceVertex3, false);
					VERIFY_EXPRESSION(vecFaceVertex3.size() == 3);

					long iVertex3 = atol(vecFaceVertex3[0].c_str()) - 1;
					double dX3 = m_vecVertices[(iVertex3 * 3) + 0];
					double dY3 = m_vecVertices[(iVertex3 * 3) + 1];
					double dZ3 = m_vecVertices[(iVertex3 * 3) + 2];

					bool bFilter = false;
					if ((dX1 >= dXMin) && (dX1 <= dXMax) &&
						(dX2 >= dXMin) && (dX2 <= dXMax) &&
						(dX3 >= dXMin) && (dX3 <= dXMax)) {
						_vector3f v1(dX1, dY1, dZ1);
						_vector3f v2(dX2, dY2, dZ2);
						_vector3f normal = v2.cross(v1);
						if ((normal.getX() < 0) && !m_mapFaceFilter.at(iFace + 1)) {
							bFilter = true;
						}
					}

					m_mapFaceFilter[iFace + 1] = bFilter;
				} // for (size_t iFace = ...
			} // Right
		}
		// Front
	}

	bool _cropping::isFaceFiltered(int64_t iFace) const
	{
		return m_mapFaceFilter.at(iFace);
	}
};
