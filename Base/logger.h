#define _IMGDISP_SOURCE_FILE_LOGGER_H
#pragma once

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

#include <string>

#include <processthreadsapi.h>
#include <sysinfoapi.h>
#include <timezoneapi.h>



/************************
 Using
*************************/
using std::string;


/************************
 DLL Export/Import define
*************************/
#ifdef DLL_EXPORT_MODE
#define DLLEXPORT			__declspec(dllexport)
#else
#define DLLEXPORT			__declspec(dllimport)
#endif


namespace Logging
{
	inline char* GetSystemTimeStr(char* dst, size_t len, char td = ':')
	{
		if (dst == nullptr)
			return nullptr;

		FILETIME ft;
		GetSystemTimeAsFileTime(&ft);
		SYSTEMTIME st;
		FileTimeToSystemTime(&ft, &st);

		char fmt[32] = "";
		HRESULT hr = E_FAIL;
		hr = StringCchPrintfA(fmt, 32, "%%u-%%02u-%%02u-%%u%c%%02u%c%%02u.%%03u", td, td);
		if (FAILED(hr))
			return nullptr;
		//string fmt = (string)"%u-%02u-%02u-%u" + td + "%02u" + td + "%02u.%03u";
		hr = StringCchPrintfA(dst, len,
			fmt,
			st.wYear, st.wMonth, st.wDay,
			st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
		if (FAILED(hr))
			return nullptr;

		return dst;
	}

	inline char* GetSystemTimeStr2(char* dst, size_t len)
	{
		if (dst == nullptr)
			return nullptr;

		FILETIME ft;
		GetSystemTimeAsFileTime(&ft);
		SYSTEMTIME st;
		FileTimeToSystemTime(&ft, &st);

		HRESULT hr = E_FAIL;
		hr = StringCchPrintfA(dst, len,
			"%u%02u%02u%u%02u%02u%03u",
			st.wYear, st.wMonth, st.wDay,
			st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
		if (FAILED(hr))
			return nullptr;

		return dst;
	}

	char* GetDirFromPath(const char* file, char* dir, size_t len);

	class DLLEXPORT Logger
	{
	public:
		static bool		__bPrepared;
		static int		Prepare();
		static Logger	_logger;

		static int		OpenLogger(const string& file);
		static int		OpenLogger(const char* file);
		static int		CloseLogger();
		static bool		IsLoggerOpen();
		static int		Log0(const string& msg);
		static int		Log0(const char* msg);
		static int		Log0(const char* h, const char* msg);
		static int		GetFile0(char* buf, size_t len);

	protected:
		FILE* _fl = nullptr;
		char			_fstr[256] = "";

	private:
		Logger(Logger& log);
		Logger& operator = (Logger& log);
	public:
		Logger();
		Logger(Logger&& log) noexcept;
		Logger& operator = (Logger&& log) noexcept;
		~Logger();

		int Open(const char* file);
		int Open(const string& file);
		int Close();
		inline bool IsOpen() const;
		int Log(const string& msg);
		int Log(const char* msg);
		int Log(const char* h, const char* msg);
		int GetFile(char* buf, size_t len) const;
	};
}

#undef _IMGDISP_SOURCE_FILE_LOGGER_H