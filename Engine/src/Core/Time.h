#pragma once
#include <chrono>
#include "gnsAPI.h"

namespace gns
{
	class Application;
	class Time
	{
		friend class  gns::Application;
	public:
		static GEN_API float GetDelta() { return m_deltaTime; }
		inline static int64_t GetNow()
		{
			 const auto now = (std::chrono::high_resolution_clock::now().time_since_epoch());
			 return now.count();
		}
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