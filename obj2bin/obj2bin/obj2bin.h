#pragma once

#include "engine.h"

// ************************************************************************************************
#ifdef _WINDOWS
#ifdef _USRDLL
#define DECSPEC __declspec(dllexport)  
#else
#define DECSPEC __declspec(dllimport) 
#endif // _USRDLL
#define STDCALL __stdcall
#else
#define DECSPEC /*nothing*/
#define STDCALL /*nothing*/
#endif // _WINDOWS

// ************************************************************************************************
/* C interface */
#ifdef __cplusplus
extern "C" {
#endif
	void DECSPEC STDCALL SetOB2BINOptions(void* pLogCallback);

	void DECSPEC STDCALL SaveAsBIN(const char* szInputFile, const char* szOutputFile);
#ifdef __cplusplus
};
#endif
