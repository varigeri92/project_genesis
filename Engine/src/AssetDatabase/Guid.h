#pragma once
#include "../Random.h"

namespace gns::core
{
	typedef uint64_t guid;
	class Guid
	{
	public:
		static guid GetNewGuid()
		{
			return Random::Get<guid>();
		}
	};
}
