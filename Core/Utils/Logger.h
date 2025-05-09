#pragma once
#include <fstream>
#include <memory>
#include <filesystem>
#include <chrono>

static const std::filesystem::path log_folder = std::filesystem::path( "Logs" );

class Logger
{
public:	
	enum Level : unsigned char
	{
		Info = 0,
		Debug,
		Warn,
		Error,
		Trace
	};
	
	explicit Logger( const std::filesystem::path& filepath, Level inital = Level::Info )
		: m_LogLevel(inital)
	{
		namespace fs = std::filesystem;
		if ( !fs::is_directory( log_folder ) || !fs::exists( log_folder ) )
			fs::create_directory( log_folder );

#ifdef DEBUG	// Force minimum level to debug on all loggers
		if ( m_LogLevel < Level::Debug )
			m_LogLevel = Level::Debug;
#endif // DEBUG

		m_file = std::make_shared<std::ofstream>( log_folder / filepath, std::ios::trunc);
		
		if ( !*m_file )
			throw std::runtime_error( "Cannot open log file " + filepath.string() );
	}

	template<typename... Args>
	void Log( Level LogLvl, std::format_string<Args...> fmt, Args&&... args ) const
	{
		using namespace std::chrono;

		if (m_LogLevel < LogLvl)
			return;
		
		auto now = time_point_cast<seconds>(system_clock::now());
		std::string final_msg = std::format( 
			"[{0} | {1:%H:%M:%S}]: {2}", 
			GetLevelName( LogLvl ), 
			now,
			std::format( fmt, std::forward<Args>( args )... )
		);
		*m_file << final_msg << "\n";
	}

	template<typename... Args> void info( std::format_string<Args...> fmt, Args&&... args ) const { Log( Level::Info, fmt, std::forward<Args>( args )... ); }
	template<typename... Args> void debug( std::format_string<Args...> fmt, Args&&... args ) const { Log( Level::Debug, fmt, std::forward<Args>( args )... ); }
	template<typename... Args> void warn( std::format_string<Args...> fmt, Args&&... args ) const { Log( Level::Warn, fmt, std::forward<Args>( args )... ); }
	template<typename... Args> void error( std::format_string<Args...> fmt, Args&&... args ) const { Log( Level::Error, fmt, std::forward<Args>( args )... ); }

	void SetLvl( Level LogLvl )
	{
		m_LogLevel = LogLvl;
	}

private:
	static constexpr std::string GetLevelName( Level LogLevel )
	{
		switch ( LogLevel )
		{
		case Logger::Info:
			return "Info ";
		case Logger::Debug:
			return "Debug";
		case Logger::Warn:
			return "Warn ";
		case Logger::Error:
			return "Error";
		case Logger::Trace:
			return "Trace";
		default:
			return " ??? ";
		}
	}

private:
	std::shared_ptr<std::ofstream> m_file;
	Level m_LogLevel;
};