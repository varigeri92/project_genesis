#pragma once

namespace gns::rendering
{
	class Device;

	class IDisposeable
	{
	public:
		virtual ~IDisposeable() = default;
		virtual void Dispose(Device* device) = 0;
	};
}