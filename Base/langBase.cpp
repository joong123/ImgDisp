#define _IMGDISP_SOURCE_FILE_LANGBASE_CPP

#include "pch.h"

#define DLL_EXPORT_MODE
#include "langBase.h"


namespace bse
{

	/************************
	 Ticker
	*************************/
	Ticker::freq_t Ticker::f = 0;
	bool Ticker::__bPrepared = Ticker::Prepare();

	bool Ticker::Prepare()
	{
		LARGE_INTEGER freq = { 0 };
		QueryPerformanceFrequency(&freq);
		f = freq.QuadPart;

		Logger::Log0("Ticker: Prepared, freq:" + to_string(freq.QuadPart));
		__bPrepared = true;

		return __bPrepared;
	}

}

#undef _IMGDISP_SOURCE_FILE_LANGBASE_CPP