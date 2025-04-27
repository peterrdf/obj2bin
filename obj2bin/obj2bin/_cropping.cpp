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
		, m_mapVertex2Faces()
		, m_setFaceNeighborsAdded()
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

		for (size_t iFace = 0; iFace < m_vecFaces.size(); iFace++) {
			m_mapFaceFilter[iFace] = true;

			vector<string> vecTokens;
			_string::split(m_vecFaces[iFace], " ", vecTokens, false);
			VERIFY_EXPRESSION(vecTokens.size() == 4);

			for (size_t iFaceVertex = 1; iFaceVertex < vecTokens.size(); iFaceVertex++) {
				vector<string> vecFaceVertex;
				_string::split(vecTokens[iFaceVertex], "/", vecFaceVertex, false);
				VERIFY_EXPRESSION(vecFaceVertex.size() == 3);

				int64_t iVertexIndex = atol(vecFaceVertex[0].c_str()) - 1;

				auto itVertex2Faces = m_mapVertex2Faces.find(iVertexIndex);
				if (itVertex2Faces != m_mapVertex2Faces.end()) {
					itVertex2Faces->second.push_back(iFace);
				} else {
					m_mapVertex2Faces[iVertexIndex] = vector<int64_t>{ int64_t(iFace) };
				}
			}
		} // for (size_t iFace = ...

		//
		// Find nearest vertex
		// 

		//double dCenterX = (m_dBBXMin + m_dBBXMax) / 2.;
		//double dCenterY = (m_dBBYMin + m_dBBYMax) / 2.;
		//double dCenterZ = (m_dBBZMin + m_dBBZMax) / 2.;

		//int64_t iNearestVertex = -1;
		//double dMinDistance = DBL_MAX;
		//int64_t iNearestVertexMaxX = -1;
		//double dMinDistanceMaxX = DBL_MAX;
		//double dMinDistanceMaxY = DBL_MAX;
		//for (size_t iVertex = 0; iVertex < m_vecVertices.size() / 3; iVertex += 3) {
		//	double dDistance = sqrt(
		//		pow(dCenterX - m_vecVertices[(iVertex * 3) + 0], 2.) +
		//		pow(dCenterY - m_vecVertices[(iVertex * 3) + 1], 2.) +
		//		pow(dCenterZ - m_vecVertices[(iVertex * 3) + 2], 2.));

		//	if (dDistance < dMinDistance) {
		//		dMinDistance = dDistance;
		//		iNearestVertex = iVertex;
		//	}

		//	// MaxY
		//	auto itVertex2Faces = m_mapVertex2Faces.find(iVertex);
		//	VERIFY_EXPRESSION(itVertex2Faces != m_mapVertex2Faces.end());

		//	for (auto iFace : itVertex2Faces->second) {
		//		vector<string> vecTokens;
		//		_string::split(m_vecFaces[iFace], " ", vecTokens, false);
		//		VERIFY_EXPRESSION(vecTokens.size() == 4);

		//		for (size_t iFaceVertex = 1; iFaceVertex < vecTokens.size(); iFaceVertex++) {
		//			vector<string> vecFaceVertex;
		//			_string::split(vecTokens[iFaceVertex], "/", vecFaceVertex, false);
		//			VERIFY_EXPRESSION(vecFaceVertex.size() == 3);

		//			if (iVertex == atol(vecFaceVertex[0].c_str()) - 1) {
		//				long iNormalIndex = atol(vecFaceVertex[2].c_str()) - 1;

		//				// X
		//				{
		//					if (m_vecNormals[(iNormalIndex * 3) + 0] < 0) {
		//						if (dDistance < dMinDistanceMaxX) {
		//							dMinDistanceMaxX = dDistance;
		//						}
		//					}
		//				}

		//				// Y
		//				{							
		//					if (m_vecNormals[(iNormalIndex * 3) + 1] > 0) {
		//						if (dDistance < dMinDistanceMaxY) {
		//							dMinDistanceMaxY = dDistance;
		//						}
		//					}
		//				}						
		//			}
		//		}
		//	}
		//}

		//
		// Min Y
		//

		double dCenterX = (m_dBBXMin + m_dBBXMax) / 2.;
		double dCenterY = (m_dBBYMin + m_dBBYMax) / 2.;
		double dCenterZ = (m_dBBZMin + m_dBBZMax) / 2.;

		int64_t iNearestVertex = -1;
		double dMinDistancePlusY = DBL_MAX;		
		double dMinDistanceMinusY = DBL_MAX;
		for (size_t iVertex = 0; iVertex < m_vecVertices.size() / 3; iVertex += 3) {
			double dDistance = sqrt(
				pow(dCenterX - m_vecVertices[(iVertex * 3) + 0], 2.) +
				pow(dCenterY - m_vecVertices[(iVertex * 3) + 1], 2.) +
				pow(dCenterZ - m_vecVertices[(iVertex * 3) + 2], 2.));

			// MaxY
			auto itVertex2Faces = m_mapVertex2Faces.find(iVertex);
			VERIFY_EXPRESSION(itVertex2Faces != m_mapVertex2Faces.end());

			for (auto iFace : itVertex2Faces->second) {
				vector<string> vecTokens;
				_string::split(m_vecFaces[iFace], " ", vecTokens, false);
				VERIFY_EXPRESSION(vecTokens.size() == 4);

				for (size_t iFaceVertex = 1; iFaceVertex < vecTokens.size(); iFaceVertex++) {
					vector<string> vecFaceVertex;
					_string::split(vecTokens[iFaceVertex], "/", vecFaceVertex, false);
					VERIFY_EXPRESSION(vecFaceVertex.size() == 3);

					if (iVertex == atol(vecFaceVertex[0].c_str()) - 1) {
						long iNormalIndex = atol(vecFaceVertex[2].c_str()) - 1;

						// Y+
						{
							if (m_vecNormals[(iNormalIndex * 3) + 1] > 0) {
								if (dDistance < dMinDistancePlusY) {
									dMinDistancePlusY = dDistance;
									iNearestVertex = iVertex;
								}
							}
						}

						// Y-
						{
							if (m_vecNormals[(iNormalIndex * 3) + 1] < 0) {
								if (dDistance < dMinDistanceMinusY) {
									dMinDistanceMinusY = dDistance;
									iNearestVertex = iVertex;
								}
							}
						}
					}
				} // for (size_t iFaceVertex = ...
			} // for (auto iFace : 
		} // for (size_t iVertex = ...

		//
		// Min X
		//

		const double STEP = (m_dBBYMax - m_dBBYMin) * .1; // 10%

		set<double> setDistancesX;
		for (double dY = dMinDistanceMinusY; dY < ((m_dBBYMin + m_dBBYMax) / 2.) + dMinDistancePlusY; dY += STEP) {
			double dCenterX = (m_dBBXMin + m_dBBXMax) / 2.;
			double dCenterY = dY;
			double dCenterZ = (m_dBBZMin + m_dBBZMax) / 2.;

			double dMinDistanceMaxX = DBL_MAX;
			for (size_t iVertex = 0; iVertex < m_vecVertices.size() / 3; iVertex += 3) {
				double dDistance = sqrt(
					pow(dCenterX - m_vecVertices[(iVertex * 3) + 0], 2.) +
					pow(dCenterY - m_vecVertices[(iVertex * 3) + 1], 2.) +
					pow(dCenterZ - m_vecVertices[(iVertex * 3) + 2], 2.));

				// MaxY
				auto itVertex2Faces = m_mapVertex2Faces.find(iVertex);
				VERIFY_EXPRESSION(itVertex2Faces != m_mapVertex2Faces.end());

				for (auto iFace : itVertex2Faces->second) {
					vector<string> vecTokens;
					_string::split(m_vecFaces[iFace], " ", vecTokens, false);
					VERIFY_EXPRESSION(vecTokens.size() == 4);

					for (size_t iFaceVertex = 1; iFaceVertex < vecTokens.size(); iFaceVertex++) {
						vector<string> vecFaceVertex;
						_string::split(vecTokens[iFaceVertex], "/", vecFaceVertex, false);
						VERIFY_EXPRESSION(vecFaceVertex.size() == 3);

						if (iVertex == atol(vecFaceVertex[0].c_str()) - 1) {
							long iNormalIndex = atol(vecFaceVertex[2].c_str()) - 1;

							// X
							{
								if (m_vecNormals[(iNormalIndex * 3) + 0] > 0) {
									if (dDistance < dMinDistanceMaxX) {
										dMinDistanceMaxX = dDistance;
									}
								}
							}
						}
					} // for (size_t iFaceVertex = ...
				} // for (auto iFace : ...

				setDistancesX.insert(dMinDistanceMaxX);
			} // for (auto iVertex : ...
		} // for (double dY = ...

		VERIFY_EXPRESSION(iNearestVertex != -1);

		auto itVertex2Faces = m_mapVertex2Faces.find(iNearestVertex);
		VERIFY_EXPRESSION(itVertex2Faces != m_mapVertex2Faces.end());

		vector<int64_t> vecFaceNeighbors = itVertex2Faces->second;
		while (!vecFaceNeighbors.empty()) {
			vector<int64_t> vecFaceChildrenNeighbors;
			for (auto iFace : vecFaceNeighbors) {
				if (m_setFaceNeighborsAdded.find(iFace) != m_setFaceNeighborsAdded.end()) {
					continue;
				}
				m_setFaceNeighborsAdded.insert(iFace);

				m_mapFaceFilter[iFace] = false;
				getFaceNeighbors(iFace, vecFaceChildrenNeighbors);
			}

			vecFaceNeighbors = vecFaceChildrenNeighbors;
			vecFaceChildrenNeighbors.clear();
		}		

		cout << "";

		// Front
		{
			// Left
			//{
			//	double dXMin = m_dBBXMin;
			//	double dXMax = (m_dBBXMin + m_dBBXMax) / 2.;

			//	for (size_t iFace = 0; iFace < m_vecFaces.size(); iFace++) {
			//		vector<string> vecTokens;
			//		_string::split(m_vecFaces[iFace], " ", vecTokens, false);
			//		VERIFY_EXPRESSION(vecTokens.size() == 4);

			//		// Vertex 1
			//		vector<string> vecFaceVertex1;
			//		_string::split(vecTokens[1], "/", vecFaceVertex1, false);
			//		VERIFY_EXPRESSION(vecFaceVertex1.size() == 3);

			//		long iVertex1 = atol(vecFaceVertex1[0].c_str()) - 1;
			//		double dX1 = m_vecVertices[(iVertex1 * 3) + 0];
			//		double dY1 = m_vecVertices[(iVertex1 * 3) + 1];
			//		double dZ1 = m_vecVertices[(iVertex1 * 3) + 2];

			//		// Vertex 2
			//		vector<string> vecFaceVertex2;
			//		_string::split(vecTokens[2], "/", vecFaceVertex2, false);
			//		VERIFY_EXPRESSION(vecFaceVertex2.size() == 3);

			//		long iVertex2 = atol(vecFaceVertex2[0].c_str()) - 1;
			//		double dX2 = m_vecVertices[(iVertex2 * 3) + 0];
			//		double dY2 = m_vecVertices[(iVertex2 * 3) + 1];
			//		double dZ2 = m_vecVertices[(iVertex2 * 3) + 2];

			//		// Vertex 3
			//		vector<string> vecFaceVertex3;
			//		_string::split(vecTokens[3], "/", vecFaceVertex3, false);
			//		VERIFY_EXPRESSION(vecFaceVertex3.size() == 3);

			//		long iVertex3 = atol(vecFaceVertex3[0].c_str()) - 1;
			//		double dX3 = m_vecVertices[(iVertex3 * 3) + 0];
			//		double dY3 = m_vecVertices[(iVertex3 * 3) + 1];
			//		double dZ3 = m_vecVertices[(iVertex3 * 3) + 2];

			//		bool bFilter = false;
			//		if ((dX1 >= dXMin) && (dX1 <= dXMax) &&
			//			(dX2 >= dXMin) && (dX2 <= dXMax) &&
			//			(dX3 >= dXMin) && (dX3 <= dXMax)) {
			//			_vector3f v1(dX1, dY1, dZ1);
			//			_vector3f v2(dX2, dY2, dZ2);
			//			_vector3f normal = v2.cross(v1);
			//			if (normal.getX() > 0) {
			//				bFilter = true;
			//			}
			//		}

			//		m_mapFaceFilter[iFace + 1] = bFilter;
			//	} // for (size_t iFace = ...
			//} // Left

			// Right
			//{
			//	double dXMin = (m_dBBXMin + m_dBBXMax) / 2.;
			//	double dXMax = m_dBBXMax;

			//	for (size_t iFace = 0; iFace < m_vecFaces.size(); iFace++) {
			//		vector<string> vecTokens;
			//		_string::split(m_vecFaces[iFace], " ", vecTokens, false);
			//		VERIFY_EXPRESSION(vecTokens.size() == 4);

			//		// Vertex 1
			//		vector<string> vecFaceVertex1;
			//		_string::split(vecTokens[1], "/", vecFaceVertex1, false);
			//		VERIFY_EXPRESSION(vecFaceVertex1.size() == 3);

			//		long iVertex1 = atol(vecFaceVertex1[0].c_str()) - 1;
			//		double dX1 = m_vecVertices[(iVertex1 * 3) + 0];
			//		double dY1 = m_vecVertices[(iVertex1 * 3) + 1];
			//		double dZ1 = m_vecVertices[(iVertex1 * 3) + 2];

			//		long iNormal1 = atol(vecFaceVertex1[2].c_str()) - 1;

			//		// Vertex 2
			//		vector<string> vecFaceVertex2;
			//		_string::split(vecTokens[2], "/", vecFaceVertex2, false);
			//		VERIFY_EXPRESSION(vecFaceVertex2.size() == 3);

			//		long iVertex2 = atol(vecFaceVertex2[0].c_str()) - 1;
			//		double dX2 = m_vecVertices[(iVertex2 * 3) + 0];
			//		double dY2 = m_vecVertices[(iVertex2 * 3) + 1];
			//		double dZ2 = m_vecVertices[(iVertex2 * 3) + 2];

			//		long iNormal2 = atol(vecFaceVertex2[2].c_str()) - 1;

			//		// Vertex 3
			//		vector<string> vecFaceVertex3;
			//		_string::split(vecTokens[3], "/", vecFaceVertex3, false);
			//		VERIFY_EXPRESSION(vecFaceVertex3.size() == 3);

			//		long iVertex3 = atol(vecFaceVertex3[0].c_str()) - 1;
			//		double dX3 = m_vecVertices[(iVertex3 * 3) + 0];
			//		double dY3 = m_vecVertices[(iVertex3 * 3) + 1];
			//		double dZ3 = m_vecVertices[(iVertex3 * 3) + 2];

			//		long iNormal3 = atol(vecFaceVertex3[2].c_str()) - 1;

			//		bool bFilter = false;
			//		if ((dX1 >= dXMin) && (dX1 <= dXMax) &&
			//			(dX2 >= dXMin) && (dX2 <= dXMax) &&
			//			(dX3 >= dXMin) && (dX3 <= dXMax)) {
			//			double dN1x = m_vecNormals[(iNormal1 * 3) + 0];
			//			double dN2x = m_vecNormals[(iNormal2 * 3) + 0];
			//			double dN3x = m_vecNormals[(iNormal3 * 3) + 0];
			//			if ((dN1x < 0) || (dN2x < 0) || (dN3x < 0)) {
			//				bFilter = true;
			//			}
			//		}

			//		m_mapFaceFilter[iFace + 1] = bFilter;
			//	} // for (size_t iFace = ...
			//} // Right
		}
		// Front
	}

	void _cropping::getFaceNeighbors(int64_t iFace/*0-based*/, vector<int64_t>& vecFaceNeighbors/*0-based*/)
	{
		vector<string> vecTokens;
		_string::split(m_vecFaces[iFace], " ", vecTokens, false);
		VERIFY_EXPRESSION(vecTokens.size() == 4);

		// Vertex 1
		{
			vector<string> vecFaceVertex1;
			_string::split(vecTokens[1], "/", vecFaceVertex1, false);
			VERIFY_EXPRESSION(vecFaceVertex1.size() == 3);

			long iVertex1 = atol(vecFaceVertex1[0].c_str()) - 1;
			
			auto itVertex2Faces = m_mapVertex2Faces.find(iVertex1);
			VERIFY_EXPRESSION(itVertex2Faces != m_mapVertex2Faces.end());

			if ((m_vecVertices[(iVertex1 * 3) + 1] < ((m_dBBYMin + m_dBBYMax) / 2.) + 0.03))
			vecFaceNeighbors.insert(vecFaceNeighbors.end(), itVertex2Faces->second.begin(), itVertex2Faces->second.end());
		}		

		// Vertex 2
		{
			vector<string> vecFaceVertex2;
			_string::split(vecTokens[2], "/", vecFaceVertex2, false);
			VERIFY_EXPRESSION(vecFaceVertex2.size() == 3);

			long iVertex2 = atol(vecFaceVertex2[0].c_str()) - 1;

			auto itVertex2Faces = m_mapVertex2Faces.find(iVertex2);
			VERIFY_EXPRESSION(itVertex2Faces != m_mapVertex2Faces.end());

			if ((m_vecVertices[(iVertex2 * 3) + 1] < ((m_dBBYMin + m_dBBYMax) / 2.) + 0.03))
			vecFaceNeighbors.insert(vecFaceNeighbors.end(), itVertex2Faces->second.begin(), itVertex2Faces->second.end());
		}		

		// Vertex 3
		{
			vector<string> vecFaceVertex3;
			_string::split(vecTokens[3], "/", vecFaceVertex3, false);
			VERIFY_EXPRESSION(vecFaceVertex3.size() == 3);

			long iVertex3 = atol(vecFaceVertex3[0].c_str()) - 1;

			auto itVertex2Faces = m_mapVertex2Faces.find(iVertex3);
			VERIFY_EXPRESSION(itVertex2Faces != m_mapVertex2Faces.end());

			if ((m_vecVertices[(iVertex3 * 3) + 1] < ((m_dBBYMin + m_dBBYMax) / 2.) + 0.03))
			vecFaceNeighbors.insert(vecFaceNeighbors.end(), itVertex2Faces->second.begin(), itVertex2Faces->second.end());
		}		
	}

	bool _cropping::isFaceFiltered(int64_t iFace/*0-based*/) const
	{
		return m_mapFaceFilter.at(iFace);
	}
};
