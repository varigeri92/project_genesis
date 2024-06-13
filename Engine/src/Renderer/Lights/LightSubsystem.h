#pragma once

namespace gns
{
class LightSubsystem
{
public:
	LightSubsystem() = default;
	~LightSubsystem() = default;
	void* CalculateLightData(void* sceneDataBuffer, size_t& buffersize);
};
}