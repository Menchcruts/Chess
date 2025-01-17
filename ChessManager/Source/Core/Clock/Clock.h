#pragma once
#include <chrono>

struct Clock
{
	std::chrono::steady_clock::duration TimeLeft = std::chrono::milliseconds( 3661000 );	// 01:01:01
	//std::chrono::milliseconds TimeLeft = std::chrono::milliseconds( 3661000 ); // 01:01:01
	std::chrono::steady_clock::time_point LastPoll = std::chrono::steady_clock::now();


	Clock() = default;
	Clock( std::chrono::milliseconds StartTime );

	void SetTimeLeft( std::chrono::milliseconds Time );
	std::chrono::milliseconds GetTimeLeft() const;
	void Update();
	std::chrono::milliseconds Poll();
	void UpdateLastPoll();
};