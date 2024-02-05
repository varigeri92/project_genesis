#pragma once
#include "gnsAPI.h"
namespace gns::rendering
{
	class Device;

	class IDisposeable
	{
	public:
		virtual ~IDisposeable() = default;
		virtual GEN_API void Dispose(Device* device) = 0;
	};
}