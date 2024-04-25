#pragma once
#include <chrono>
#include "api.h"

namespace gns
{
	class Engine;
class Time
{
	friend class Engine;
	static float m_deltaTime;
public:
	static GNS_API float GetDelta() { return m_deltaTime; }
	static int64_t GetNow();
private:
	static std::chrono::time_point<std::chrono::steady_clock> m_startTime;
	static void StartFrameTime();
	static void EndFrameTime();
	
};
}
