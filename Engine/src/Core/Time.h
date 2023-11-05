#pragma once
#include <chrono>

namespace gns
{
	class Application;
	class Time
	{
		friend class  gns::Application;
	public:
		static float GetDelta() { return m_deltaTime; }
	private:
		inline static float m_deltaTime = 0;
		inline static std::chrono::time_point<std::chrono::steady_clock> m_startTime;
		static void StartFrameTime()
		{
			
			m_startTime = std::chrono::high_resolution_clock::now();
			
		}
		static void EndFrameTime()
		{
			auto currentTime = std::chrono::high_resolution_clock::now();
			m_deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - m_startTime).count();
		}
	};
}