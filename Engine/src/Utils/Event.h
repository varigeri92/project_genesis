#pragma once
#include <functional>
#include <map>

#include "api.h"
#include "Log.h"
#include "../Utils/Random.h"

namespace gns
{
	class EventFunctionBase
	{
	public:
		virtual GNS_API ~EventFunctionBase() = default;
	};

	template<class T, typename... Args>
	class EventFunction : public EventFunctionBase
	{
		using functionType = std::function <T(Args&& ...)>;
	public:
		functionType m_function;
		size_t m_functionID;
		EventFunction(functionType function) : m_function(function)
		{
			m_functionID = Random::Get<size_t>();
		}

		T Call(Args&& ... args )
		{
			return m_function(std::forward<Args>(args) ...);
		}
	};

	template<class T, typename... Args>
	class Event
	{
		using functionType = std::function<T(Args&& ...)>;

		std::map<size_t, functionType> EventMap;
	private:
		size_t getAddress(const functionType& f)
		{
			const size_t address = reinterpret_cast<size_t>(std::addressof(f));
			return address;
		}

	public:
		template<class EventFunc, typename = std::enable_if_t<std::is_same_v<EventFunc, EventFunction<T, Args ... >>>>
		void Subscribe(EventFunctionBase* function)
		{
			EventFunction<T, Args ...>* f = dynamic_cast<EventFunction<T, Args ...>*>(function);
			if (f == nullptr)
			{
				return;
			}
			EventMap[f->m_functionID] = f->m_function;
		}

		void Dispatch(Args ... args)
		{
			for (auto it = EventMap.begin(); it != EventMap.end(); it++)
			{
				it->second(std::forward<Args>(args) ... );
			}
		}

		template<class EventFunc, typename = std::enable_if_t<std::is_same_v<EventFunc, EventFunction<T, Args ... >>>>
		void RemoveListener(EventFunctionBase* function)
		{
			EventFunction<T, Args ...>* f = dynamic_cast<EventFunction<T, Args ...>*>(function);
			EventMap.erase(f->m_functionID);
		}
	};
}
