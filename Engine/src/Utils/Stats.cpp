#include "gnspch.h"
#include "Stats.h"
#include <chrono>
void Stats::StartTimer()
{
	m_startTime = std::chrono::high_resolution_clock::now();
}

void Stats::StopTimer()
{
	auto currentTime = std::chrono::high_resolution_clock::now();
	time = std::chrono::duration<double, std::chrono::seconds::period>(currentTime - m_startTime).count();
}
