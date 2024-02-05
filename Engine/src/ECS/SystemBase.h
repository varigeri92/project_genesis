#pragma once
#include "../Application.h"

namespace gns::core
{
	class SystemsApi;
	class SystemBase
	{
		friend class SystemsApi;
	public:
		GEN_API SystemBase(std::string name);
		virtual ~SystemBase() = default;

		std::string name;
		guid guid;
		size_t index;
	protected:
		GEN_API virtual void OnCreate() = 0;
		GEN_API virtual void OnUpdate() = 0;
		GEN_API virtual void OnDestroy() = 0;
	};
}
