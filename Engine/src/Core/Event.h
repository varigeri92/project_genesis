#pragma once
#include <functional>
#include <vector>
#include "../Application.h"

template<class T, typename... Args>
class Event
{
private:
	std::vector<std::function<T(Args&& ...)>> queue;
public:

	void Subscribe(const std::function<T(Args&& ...)>& function)
	{
		queue.push_back(function);
	}
	void Dispatch(Args&& ... args)
	{
		for (size_t i = 0; i < queue.size(); i++)
		{
			queue[i](std::forward<Args>(args) ... );
		}
		
	}

	bool HasFunction(const std::function<T(Args&& ...)>& function)
	{
		for (const auto& storedFunc : queue) {
			if (function.target_type() == storedFunc.target_type() && function.target<std::function<T(Args&& ...)>>() 
				== storedFunc.target<std::function<T(Args&& ...)>>()) {
				return true;
			}
		}
		return false;
	}

	void RemoveListener(const std::function<T(Args&& ...)>& function)
	{
		size_t index = 0;
		bool found = false;
		for (const auto& storedFunc : queue) {
			if (function.target_type() == storedFunc.target_type() && function.target<std::function<T(Args&& ...)>>()
				== storedFunc.target<std::function<T(Args&& ...)>>()) {
				found = true;
				break;
			}
			index++;
		}
		if(found)
		{
			auto it = queue.begin() + index;
			queue.erase(it);
		}
	}
};
