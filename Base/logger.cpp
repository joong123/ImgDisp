#include "pch.h"

#define _IMGDISP_SOURCE_FILE_LOGGER_CPP

#define DLL_EXPORT_MODE
#include "logger.h"

#include <io.h>
#include <direct.h>


namespace Logging
{
	char* GetDirFromPath(const char* file, char* dir, size_t len)
	{
		if (nullptr == file || nullptr == dir)
			return nullptr;

		size_t lenPath = 0;
		if (FAILED(StringCchLengthA(file, MAX_PATH, &lenPath)))
			return nullptr;
		if (len <= 0)
			return nullptr;
		size_t idxSl = len;
		for (size_t i = len - 1; i > 0; --i)
		{
			if (file[i] == '\\')
			{
				idxSl = i;
				break;
			}
		}
		if (idxSl == len)
		{
			// TODO
			//if (file[0] == '\\')
			//	idxSl = 0;
		}

		if (idxSl == len)
		{
			return nullptr;
		}

		StringCchCopyNA(dir, len, file, idxSl);
		return dir;
	}

	bool Logger::__bPrepared = false;
	Logger Logger::_logger = Logger();

	bool Logger::IsLoggerOpen()
	{
		return _logger.IsOpen();
	}

	int Logger::Prepare()
	{
		// open logger
		char timestr[64] = "";
		Logging::GetSystemTimeStr(timestr, 64, '-');
		char file[256] = "";
		StringCchPrintfA(file, 256, ".\\Log\\ImgDisp_Log_%s.txt", timestr);
		return Logger::OpenLogger(file);
	}

	int Logger::OpenLogger(const string& file)
	{
		return OpenLogger(file.c_str());
	}
	int Logger::OpenLogger(const char* file)
	{
		if (nullptr == file)
			return -1;

		char dir[MAX_PATH] = "";
		const char* pDir = GetDirFromPath(file, dir, MAX_PATH);
		if (nullptr != pDir)
		{
			if (0 != _access(pDir, 0))
			{
				if (0 != _mkdir(pDir))
				{
					return -2;
				}
			}
		}

		int ret = _logger.Open(file);

		if (ret > 0)
		{
			__bPrepared = true;
		}
		else
		{
			__bPrepared = false;

			/*ofstream ooo;
			ooo.open(".\\error_log.txt");
			ooo << "open file ret: " + std::to_string(ret)
				<< ", " << _logger.IsOpen() << endl;
			ooo << "open file " + file + " failed!" << endl;
			ooo.close();*/
		}

		return ret;
	}

	int Logger::CloseLogger()
	{
		if (!_logger.IsOpen())
		{
			return -1;
		}

		_logger.Log("Logger Closed!\n");
		return _logger.Close();
	}

	int Logger::Log0(const string& msg)
	{
		return _logger.Log(msg);
	}

	int Logger::Log0(const char* msg)
	{
		return _logger.Log(msg);
	}

	int Logger::Log0(const char* h, const char* msg)
	{
		return _logger.Log(h, msg);
	}

	int Logger::GetFile0(char* buf, size_t len)
	{
		return _logger.GetFile(buf, len);
	}


	Logger::Logger()
	{
	}

	Logger::Logger(Logger&& log) noexcept
	{
		_fl = log._fl;
		log._fl = nullptr;

		StringCchCopyA(_fstr, 256, log._fstr);
		StringCchCopyA(log._fstr, 256, "");
	}

	Logger& Logger::operator = (Logger&& log) noexcept
	{
		_fl = log._fl;
		log._fl = nullptr;

		StringCchCopyA(_fstr, 256, log._fstr);
		StringCchCopyA(log._fstr, 256, "");

		return *this;
	}

	Logger::~Logger()
	{
		Close();
	}

	int Logger::Open(const string& file)
	{
		return Open(file.c_str());
	}
	int Logger::Open(const char* file)
	{
		if (nullptr == file)
			return -1;

		if (IsOpen())
		{
			Close();
		}

		//_fl = new FILE;
		errno_t err = fopen_s(&_fl, file, "w");
		if (err != 0)
		{
			return -1;
		}
		if (!IsOpen())
		{
			return -2;
		}
		StringCchCopyA(_fstr, 256, file);

		return 1;
	}

	int Logger::Close()
	{
		if (IsOpen())
		{
			Log("Logger Closed!");
			fclose(_fl);
			delete _fl;
			return 1;
		}

		return 0;
	}
	bool Logger::IsOpen() const
	{
		return _fl != nullptr;
	}

	int Logger::Log(const string& msg)
	{
		return Log(msg.c_str());
	}

	int Logger::Log(const char* msg)
	{
		if (IsOpen())
		{
			char timestr[64] = "";
			Logging::GetSystemTimeStr(timestr, '-');
			return fprintf_s(_fl, "%s\t[T#%ul]\t%s\n"
				, timestr, GetCurrentThreadId(), msg);
		}

		return 0;
	}
	int Logger::Log(const char* h, const char* msg)
	{
		if (IsOpen())
		{
			char timestr[64] = "";
			Logging::GetSystemTimeStr(timestr, '-');
			return fprintf_s(_fl, "%s\t[T#%ul]\t%s%s\n"
				, timestr, GetCurrentThreadId(), h, msg);
		}

		return 0;
	}
	int Logger::GetFile(char* buf, size_t len) const
	{
		if (__bPrepared)
		{
			if (FAILED(StringCchCopyA(buf, len, _fstr)))
				return -1;
			return 1;
		}

		if (FAILED(StringCchCopyA(buf, len, "")))
			return 0;
		return 0;
	}
}

#undef _IMGDISP_SOURCE_FILE_LOGGER_CPP