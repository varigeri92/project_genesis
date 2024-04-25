#include "gnspch.h"
#include "Time.h"

std::chrono::time_point<std::chrono::steady_clock> gns::Time::m_startTime = {};
float gns::Time::m_deltaTime = 0;

int64_t gns::Time::GetNow()
{
	const auto now = (std::chrono::high_resolution_clock::now().time_since_epoch());
	return now.count();
}

void gns::Time::StartFrameTime()
{
	m_startTime = std::chrono::high_resolution_clock::now();
}

void gns::Time::EndFrameTime()
{
	auto currentTime = std::chrono::high_resolution_clock::now();
	m_deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - m_startTime).count();
}
