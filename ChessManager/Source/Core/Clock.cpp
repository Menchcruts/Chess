#include "Clock.h"


Clock::Clock()
{
    using namespace std::chrono;
    TimeLeft = 0ms;
    UpdateLastPoll();
}

Clock::Clock( std::chrono::milliseconds StartTime ) : TimeLeft(StartTime)
{
    UpdateLastPoll();
}

void Clock::SetTimeLeft( std::chrono::milliseconds Time )
{
    TimeLeft = Time;
}

std::chrono::milliseconds Clock::GetTimeLeft() const
{
    return std::chrono::milliseconds();
}

void Clock::Update()
{
    using namespace std::chrono;
    auto now = high_resolution_clock::now();
    auto diff = duration_cast<milliseconds>(LastPoll - now);
    TimeLeft -= diff;
}

void Clock::UpdateLastPoll()
{
    LastPoll = std::chrono::high_resolution_clock::now();
}
