#pragma once
#include "../Utils/Color.h"

//Transform
struct Light
{
	float intensity;
	Color color;
};

struct SpotLight
{
	float angle;
	float range;
};

struct PointLight
{
	float range;
};

struct ShadowCaster
{
	enum class ShadowType
	{
		HardShadow,
		SoftShadow
	};

	uint32_t resolution;
	ShadowType shadowType;
};
