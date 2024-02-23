#pragma once
#include <functional>
#include <vector>
#include "../Application.h"
#include "Log.h"

#define EVT_FN(retType, fnName, ...) std::function<retType()> fnName( __VA_ARGS__ )

template<class T, typename... Args>
class Event
{
private:
	std::vector<std::function<T(Args&& ...)>> queue;

	size_t getAddress(std::function<T(Args&& ...)> f) 
	{	
		using fntype = T(Args&& ...);
		fntype** fnPtr = f.template target<fntype*>();
		if (fnPtr == nullptr)
			return 0;
		return reinterpret_cast<size_t>(*fnPtr);
	}

public:

	void Subscribe(const std::function<T(Args&& ...)>& function)
	{
		size_t id = getAddress(function);
		queue.push_back(function);
		size_t id_2 = getAddress(queue[queue.size() - 1]);
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
		auto it = queue.begin();
		/*
		for (it = queue.begin(); it != queue.end(); it++) {
			if (getAddress(queue[it]) == getAddress(function))
				break;
		}
		if(it != queue.end())
			queue.erase(it);
		*/
	}
};
