#pragma once

namespace gns::rendering
{
	class Device;

	class Disposeable
	{
	public:
		virtual ~Disposeable() = default;
		virtual void Dispose(Device* device) = 0;
	};
}