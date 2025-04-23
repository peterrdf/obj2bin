#include "pch.h"
#include "_exporter.h"

// ************************************************************************************************
namespace _obj2bin
{
	// ************************************************************************************************
	_exporter::_exporter(const char* szInputFile, const char* szOutputFile)
		: _log_client()
		, m_iModel(0)
		, m_strInputFile()
		, m_strOutputFile()
		, m_iVerticesCount(0)
		, m_iFacesCount(0)
		, m_vecVertices()
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
		int iDataOffset = 0;
		if (!readHeader(iDataOffset)) {
			return;
		}

		//#todo

		// plate
		m_iVerticesCount = 8453; m_iFacesCount = 16539;

		std::ifstream binStream(m_strInputFile.c_str(), std::ios::in | std::ios::binary);
		VERIFY_EXPRESSION(binStream.good());

		binStream.seekg(iDataOffset, ios::beg);

		//#todo
		//bool big_endian = is_big_endian();

		// #todo readVertices
		{


			//#todo verices count
			for (int v = 0; v < m_iVerticesCount; v++) {
				{
					vector<double> vertex;
					vertex.resize(3);
					binStream.read((char*)vertex.data(), 3 * sizeof(double));

					m_vecVertices.insert(m_vecVertices.end(), vertex.begin(), vertex.end());

				}

				{
					vector<unsigned char> rgb;
					rgb.resize(3);
					binStream.read((char*)rgb.data(), 3 * sizeof(unsigned char));
				}

				{
					unsigned char alpha;
					binStream.read((char*)&alpha, sizeof(unsigned char));
				}

				{
					double quality;
					binStream.read((char*)&quality, sizeof(double));
				}
			}

			// #todo read faces
			{
				for (int v = 0; v < m_iFacesCount; v++) {
					unsigned char count;
					binStream.read((char*)&count, sizeof(unsigned char));

					vector<int> vertex;
					vertex.resize(count);
					binStream.read((char*)vertex.data(), count * sizeof(int));

					m_vecIndices.insert(m_vecIndices.end(), vertex.begin(), vertex.end());
					m_vecIndices.push_back(-1);
				}
			}

			binStream.close();
		}

		cout << "";

		m_iModel = CreateModel();
		assert(m_iModel != 0);

		{
			//OwlClass iClass = GetClassByName(m_iModel, "TriangleSet");
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
		}


		//#todo
		SaveModel(m_iModel, m_strOutputFile.c_str());
	}

	bool _exporter::readHeader(int& iDataOffset)
	{
		iDataOffset = 0;

		auto pReader = new _file_reader();
		if (!pReader->open(m_strInputFile.c_str())) {
			THROW_ERROR(_err::_file);
		}

		char ch = pReader->getChar();

		if (ch == EOF) {
			THROW_ERROR(_err::_file);
		}

		iDataOffset = 1;

		int iLineIndex = -1;
		string strHeaderLine;
		while (ch != EOF) {
			if ((ch == LF) || (ch == CR)) {
				iLineIndex++;
				if (iLineIndex > 100) {
					THROW_ERROR(_err::_format);
				}

				if (strHeaderLine == "end_header") {
					break;
				}

				strHeaderLine = "";

				ch = pReader->getNextChar(false);
				iDataOffset++;

				continue;
			}

			strHeaderLine += ch;

			ch = pReader->getNextChar(false);
			iDataOffset++;
		} // while (ch != EOF)

		delete pReader;

		return strHeaderLine == "end_header";
	}
};