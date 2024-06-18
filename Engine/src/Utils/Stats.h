#pragma once
#include <string>
#include "Time.h"
struct Stats
{
	std::string name;
	double time;
	std::chrono::time_point<std::chrono::steady_clock> m_startTime;

	void StartTimer();
	void StopTimer();
};