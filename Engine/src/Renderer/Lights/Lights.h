#pragma once
#include "../../Engine/src/Utils/Color.h"
#include "../../Engine/src/SystemsApi/ComponentLibrary.h"

namespace gns
{
	struct Light : public ComponentBase
	{
		Color color;
		float intensity;
		void Register() override
		{
			REGISTER_CMP(Light);
			REGISTER_FIELD(Color, color);
			REGISTER_FIELD(float, intensity);
		}
		Light() : color{ 1,1,1,1 }, intensity{1}{}
		Light(Color color, float intensity) : color{color}, intensity{ intensity }{}
	};

	struct Ambient : public ComponentBase
	{
		Color color;
		float intensity;
		void Register() override
		{
			REGISTER_CMP(Ambient);
			REGISTER_FIELD(Color, color);
			REGISTER_FIELD(float, intensity);
		}
		Ambient() : color{ 1,1,1,1 }, intensity{ 1 } {}
		Ambient(Color color, float intensity) : color{ color }, intensity{ intensity } {}
	};


	struct LightDirection : public ComponentBase
	{
		glm::vec3 forward;
		void Register() override
		{
			REGISTER_CMP(LightDirection);
			REGISTER_FIELD(glm::vec3, forward);
		}
	};

	struct SpotLight : public ComponentBase
	{
		float angle;
		float range;
		void Register() override
		{
			REGISTER_CMP(SpotLight);
			REGISTER_FIELD(float, angle);
			REGISTER_FIELD(float, range);
		}
	};

	struct PointLight : public ComponentBase
	{
		float range;
		void Register() override
		{
			REGISTER_CMP(PointLight);
			REGISTER_FIELD(float, range);
		}
	};

	struct ShadowCaster : public ComponentBase
	{
		enum class ShadowType
		{
			HardShadow,
			SoftShadow
		};
		uint32_t resolution;
		ShadowType shadowType;
		void Register() override{}
	};
}
