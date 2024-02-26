#pragma once
#include "gnsAPI.h"
namespace gns::rendering
{
	class Device;

	class IDisposeable
	{
	public:
		virtual ~IDisposeable() = default;
		virtual GNS_API void Dispose(Device* device) = 0;
		//virtual GNS_API void Dispose() = 0;
	};
}