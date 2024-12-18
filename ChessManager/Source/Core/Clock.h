#pragma once
#include <chrono>

struct Clock
{
	std::chrono::milliseconds TimeLeft;
	std::chrono::steady_clock::time_point LastPoll;

	Clock();
	Clock(std::chrono::milliseconds StartTime);

	void SetTimeLeft( std::chrono::milliseconds Time );
	std::chrono::milliseconds GetTimeLeft() const;
	void Update();
	void UpdateLastPoll();
};