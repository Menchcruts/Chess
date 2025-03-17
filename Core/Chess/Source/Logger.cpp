#include "Logger.h"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <chrono>

namespace Logging
{
	std::filesystem::path LogFilePath = "Logging";

	class Logger
	{
	private:
		std::ofstream LogFile;
		LogLevel LoggingLevel = LogLevel::Info;
		std::string GetLogLvLName( LogLevel LvL )
		{
			switch ( LvL )
			{
			case Logging::Info:
				return "Info";
			case Logging::Debug:
				return "Debug";
			case Logging::Warning:
				return "Warning";
			case Logging::Error:
				return "Error";
			case Logging::Trace:
			default:
				return "Trace";
			}
		}

	public:
		Logger()
		{
			LogFile.open( "Log.txt" );
		}
		~Logger()
		{
			LogFile.close();
		}

		void Log( std::string Message, LogLevel LvL )
		{
			if ( LogFile.is_open() && LvL <= LoggingLevel )
			{
				std::string TimeString = std::format( "{:%H:%M:%S}", std::chrono::system_clock::now() );
				std::string InfoString = std::format( "[{} / {}] {}", GetLogLvLName(LvL), TimeString, Message);
				//std::string InfoString = "";
				LogFile << InfoString << "\n";
			}
		}

		void SetLogLevel( LogLevel NewLvL )
		{
			LoggingLevel = NewLvL;
		}
	};

	Logger MainLogger;
	
	void Log( std::string Message, LogLevel LvL )
	{
		MainLogger.Log( Message, LvL );
	}
	void SetLevel( LogLevel NewLvL )
	{
		MainLogger.SetLogLevel( NewLvL );
	}
}
