#define _IMGDISP_SOURCE_FILE_LOGGER_CPP

#include "pch.h"

#define DLL_EXPORT_MODE
#include "logger.h"


namespace Logging
{

	// Global logger
	Logger g_logger = Logger();

	bool Logger::__bPrepared = Logger::Prepare();// TODO

	bool Logger::IsLoggerOpen()
	{
		return g_logger.IsOpen();
	}

	bool Logger::Prepare()
	{
		// open logger
		string file = ".\\ImgDisp_Log_" + Logging::GetSystemTimeStr('-') + ".txt";
		int ret = Logger::OpenLogger(file);

		if (ret == 1)
		{
			__bPrepared = true;
		}
		else
		{
			__bPrepared = false;

			/*ofstream ooo;
			ooo.open(".\\error_log.txt");
			ooo << "open file ret: " + std::to_string(ret)
				<< ", " << g_logger.IsOpen() << endl;
			ooo << "open file " + file + " failed!" << endl;
			ooo.close();*/
		}

		return __bPrepared;
	}

	int Logger::OpenLogger(const string& file)
	{
		int ret = g_logger.Open(file);
		g_logger.Log("Logger Opened!\n");

		return ret;
	}

	int Logger::CloseLogger()
	{
		if (!g_logger.IsOpen())
		{
			return -1;
		}

		g_logger.Log("Logger Closed!\n");
		return g_logger.Close();
	}

	int Logger::Log0(const string& msg)
	{
		if (!g_logger.IsOpen())
		{
			return -1;
		}

		int ret = g_logger.Log(msg);

		return ret;
	}

	int Logger::Log0(const string& h, const string& msg)
	{
		if (!g_logger.IsOpen())
		{
			return -1;
		}

		int ret = g_logger.Log(h + msg);

		return ret;
	}

	string Logger::GetFile0()
	{
		return g_logger.GetFile();
	}


	Logger::Logger()
	{
	}

	Logger::Logger(Logger&& log)
	{
		_out.move(std::forward<ofstream>(log._out));
	}

	Logger& Logger::operator = (Logger&& log)
	{
		_out.move(std::forward<ofstream>(log._out));

		return *this;
	}

	Logger::~Logger()
	{
		Close();
	}

	int Logger::Open(const string& file)
	{
		if (IsOpen())
		{
			Close();
		}

		_out.open(file);
		if (!IsOpen())
		{
			return -1;
		}
		_file = file;

		return 1;
	}

	int Logger::Close()
	{
		if (IsOpen())
		{
			Log("Logger Closed!");
			_out.close();
			_file.clear();
			return 1;
		}

		return 0;
	}

}

#undef _IMGDISP_SOURCE_FILE_LOGGER_CPP