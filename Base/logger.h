#define _IMGDISP_SOURCE_FILE_LOGGER_H
#pragma once

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

#include <string>
#include <fstream>


/************************
 Using
*************************/
using std::endl;
using std::string;
using std::ofstream;



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

	//class DLLEXPORT Logger;
	//extern DLLEXPORT Logger g_logger;

	inline string GetSystemTimeStr(char td = ':')
	{
		FILETIME ft;
		GetSystemTimeAsFileTime(&ft);
		SYSTEMTIME st;
		FileTimeToSystemTime(&ft, &st);

		char buf[32] = { 0 };
		string fmt = (string)"%u-%02u-%02u-%u" + td + "%02u" + td + "%02u.%03u";
		StringCchPrintfA(buf, 32, fmt.c_str(),
			st.wYear, st.wMonth, st.wDay,
			st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

		return string(buf);
	}

	inline string GetSystemTimeStr2()
	{
		FILETIME ft;
		GetSystemTimeAsFileTime(&ft);
		SYSTEMTIME st;
		FileTimeToSystemTime(&ft, &st);

		char buf[32] = { 0 };
		string fmt = (string)"%u%02u%02u%02u%02u%02u%03u";
		StringCchPrintfA(buf, 32, fmt.c_str(),
			st.wYear, st.wMonth, st.wDay,
			st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

		return string(buf);
	}

	class DLLEXPORT Logger
	{
	public:
		static bool		__bPrepared;
		static bool		Prepare();

		static int		OpenLogger(const string& file);
		static int		CloseLogger();
		static bool		IsLoggerOpen();
		static int		Log0(const string& msg);
		static int		Log0(const string& h, const string& msg);
		static string	GetFile0();

	protected:
		string			_file;
		ofstream		_out;

	private:
		Logger(Logger& log);
		Logger& operator = (Logger& log);
	public:
		Logger();
		Logger(Logger&& log);
		Logger& operator = (Logger&& log);
		~Logger();

		int Open(const string& file);
		int Close();
		inline bool IsOpen() const
		{
			return _out.is_open();
		}
		int Log(const string& msg)
		{
			if (IsOpen())
			{
				_out << GetSystemTimeStr() << "\t[T#" << GetCurrentThreadId() << "]\t" << msg << endl;

				return 1;
			}

			return 0;
		}
		string GetFile() const
		{
			if (__bPrepared)
			{
				return _file;
			}

			return "";
		}
	};


}

#undef _IMGDISP_SOURCE_FILE_LOGGER_H