#pragma once

namespace gns::rendering
{
	class Device;
	class Disposeable
	{
	public:
		virtual void Dispose(Device* device) = 0;
	};
}