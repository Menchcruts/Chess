#pragma once
#include <string>

namespace Logging
{
	enum LogLevel
	{
		Info		= 0,
		Debug		= 1,
		Warning		= 2,
		Error		= 3,
		Trace		= 4
	};

	void Log( std::string Message, LogLevel LvL = LogLevel::Info );
	void SetLevel( LogLevel NewLvL );
}