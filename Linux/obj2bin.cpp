#include "engine.h"
#include "ifcengine.h"

#include "_rdf_mvc.h"
#include "_obj2bin.h"
#include "_bin2glb.h"

#include <iostream>
#include <string>
#ifdef _WINDOWS
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#else
#include <filesystem>
namespace fs = std::filesystem;
#endif
using namespace std;

int main(int argc, char* argv[])
{
	if (argc != 3) {
		std::cout << "Error: invalid number of arguments." << "\n";

		return -1;
	}

	// Input
	std::cout << "Model: " << argv[1] << "\n";

	// Revision
	std::cout << "Revision: " << GetRevision() << "\n";

	fs::path pathModel = argv[1];
	string strExtension = pathModel.extension().string();
	std::transform(strExtension.begin(), strExtension.end(), strExtension.begin(), ::tolower);

	if (strExtension != ".obj") {
		std::cout << "Error: Unsupported file format." << "\n";
		return -1;
	}

	_c_log log(nullptr);

	// OBJ to BIN
	{		
		_obj2bin::_exporter exporter((const char*)argv[1], (const char*)argv[2], false);
		exporter.setLog(&log);
		exporter.execute();
	}

	return 0;
}