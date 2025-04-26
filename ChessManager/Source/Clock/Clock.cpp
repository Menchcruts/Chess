#include <iostream>
#include "Clock.h"


Clock::Clock( std::chrono::milliseconds StartTime ) : TimeLeft(StartTime)
{
    UpdateLastPoll();
}

void Clock::SetTimeLeft( std::chrono::milliseconds Time )
{
    TimeLeft = Time;
    UpdateLastPoll();
}

std::chrono::milliseconds Clock::GetTimeLeft() const
{
    using namespace std::chrono;
    return duration_cast<milliseconds>(TimeLeft);
}

void Clock::Update()
{
    using namespace std::chrono;

    auto now = steady_clock::now();
    TimeLeft -= (now - LastPoll);
    LastPoll = now;
}

/* Update the timer and return time left */
std::chrono::milliseconds Clock::Poll()
{
    Update();
    return GetTimeLeft();
}

/* Set last poll to now without updating the time left */
void Clock::UpdateLastPoll()
{
    LastPoll = std::chrono::steady_clock::now();
}
