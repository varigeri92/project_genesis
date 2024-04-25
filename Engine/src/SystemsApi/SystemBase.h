#pragma once
#include "api.h"

namespace gns
{
	class SystemsAPI;
	class SystemBase
	{
		friend class SystemsAPI;

		size_t typeHash;
	public:
		GNS_API SystemBase() = default;
		GNS_API virtual ~SystemBase() = default;
	public:
		GNS_API virtual void OnCreate() = 0;
		GNS_API virtual void OnUpdate(float deltaTime) = 0;
	};
}
