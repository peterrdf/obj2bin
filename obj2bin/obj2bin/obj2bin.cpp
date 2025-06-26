#include "pch.h"
#include "obj2bin.h"

#include "_log.h"
#include "_obj2bin.h"

#ifdef _WINDOWS
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#else
#include <filesystem>
namespace fs = std::filesystem;
#endif

// ************************************************************************************************
static _log_callback g_pLogCallback = nullptr;

// ************************************************************************************************
#ifdef __cplusplus
extern "C" {
#endif
	void DECSPEC STDCALL SetOBJ2BINOptions(void* pLogCallback)
	{
		g_pLogCallback = (_log_callback)pLogCallback;
	}

	void DECSPEC STDCALL SaveAsBIN(const char* szInputFile, const char* szOutputFile)
	{
		_c_log log(g_pLogCallback);

		_obj2bin::_exporter exporter(szInputFile, szOutputFile, false/*XRCulture WASM & 3DEditor*/);
		exporter.setLog(&log);
		exporter.execute();
	}
#ifdef __cplusplus
};
#endif
