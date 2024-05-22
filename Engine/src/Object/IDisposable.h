#pragma once
namespace gns
{
	class IDisposable
	{
	public:
		virtual ~IDisposable() = default;
		GNS_API virtual void Dispose() = 0;
	};
}