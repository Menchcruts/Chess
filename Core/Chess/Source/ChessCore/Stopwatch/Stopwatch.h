#pragma once
#include <iostream>
#include <chrono>
#include <string>


class StopWatch
{
private:
	std::chrono::steady_clock::time_point m_Start;
	std::string m_Name;

public:
	StopWatch() : m_Start(std::chrono::high_resolution_clock::now()), m_Name("") {}
	StopWatch(const std::string& name) : m_Start( std::chrono::high_resolution_clock::now() ), m_Name(name) {}
	~StopWatch()
	{
		if ( m_Name.size() > 0 )
		{
			std::cout << m_Name << " took " << GetMillisecondsElapsed() << "ms to complete.\n";
		}
	}

	unsigned int GetMillisecondsElapsed() const
	{
		std::chrono::steady_clock::time_point current_time = std::chrono::high_resolution_clock::now();
		std::chrono::milliseconds duration = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - m_Start);

		return static_cast<unsigned long>(duration.count());
	}

	unsigned long int GetMicrosecondsElapsed () const
	{
		std::chrono::steady_clock::time_point current_time = std::chrono::high_resolution_clock::now();
		std::chrono::microseconds duration = std::chrono::duration_cast<std::chrono::microseconds>(current_time - m_Start);

		return static_cast<unsigned long>(duration.count());
	}

	void PrintElapsed(const char* message = "") const { std::cout << message << " | Milliseconds elapsed: " << GetMillisecondsElapsed() << "ms\n"; }

	void Reset() { m_Start = std::chrono::high_resolution_clock::now(); }
};