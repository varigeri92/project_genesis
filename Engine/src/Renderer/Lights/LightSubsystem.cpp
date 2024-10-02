#include "gnspch.h"
#include "LightSubsystem.h"

#include "Lights.h"
#include "../../Level/SceneManager.h"
#include "../RenderSystem.h"
#define MAX_LIGHTS = 100;

namespace gns
{
	static SceneData sceneData = {
		{0,0,0,0},
		{0,0,0,0},
		{0,0,0,0},
		{0,0,0,0},
	};
	static SceneLights sceneLights = {
		{},
		{},
		{}
	};

	void* LightSubsystem::CalculateLightData(void* out_sceneDataBuffer, size_t& out_bufferSize)
	{
		sceneLights.directionalLights.clear();
		sceneLights.pointLights.clear();
		sceneLights.spotLights.clear();
		auto lights = SystemsAPI::GetRegistry().view<Transform, Light, LightDirection>();
		for (auto [ent, transform, light, direction] : lights.each())
		{
			float pitch = (transform.rotation.x); // Rotation around the X-axis
			float yaw = (transform.rotation.y);   // Rotation around the Y-axis
			float roll = (transform.rotation.z); // Rotation around the Z-axis

			direction.forward.x = cos(pitch) * sin(yaw);
			direction.forward.y = sin(pitch);
			direction.forward.z = cos(pitch) * cos(yaw);
			direction.forward = glm::normalize(direction.forward);
			direction.forward.y *= -1;
			direction.forward *= -1;

			sceneData.sunlightDirection = { direction.forward.x, direction.forward.y, direction.forward.z, 0.5f };
			sceneData.sunlightColor = { light.color.r, light.color.g, light.color.b, light.intensity };
			glm::vec4 dl_direction = { direction.forward.x, direction.forward.y, direction.forward.z, light.intensity };
			glm::vec4 dl_color = { light.color.r, light.color.g ,light.color.b ,light.color.a };
			sceneLights.directionalLights.emplace_back(dl_direction, dl_color);
		}

		auto ambientView = SystemsAPI::GetRegistry().view<Transform, Ambient>();
		for (auto [ent, transform, ambient] : ambientView.each())
		{
			sceneData.ambientColor = { ambient.color.r, ambient.color.g,ambient.color.b,ambient.color.a };
			sceneData.ambientColor.w = ambient.intensity;
		}

		auto pointLights = SystemsAPI::GetRegistry().view<Transform, Light, PointLight>();
		for (auto [ent, transform, light, pointLight] : pointLights.each())
		{
			glm::vec4 pl_position = { transform.position.x, transform.position.y, transform.position.z,pointLight.range };
			glm::vec4 pl_color = { light.color.r,light.color.g,light.color.b, light.intensity };
			sceneLights.pointLights.emplace_back(pl_position, pl_color);
		}

		char* bufferWithOffset = static_cast<char*>(out_sceneDataBuffer);
		size_t currentoffset = sizeof(SceneData);
		memcpy(bufferWithOffset, &sceneData, currentoffset);

		bufferWithOffset += currentoffset;
		currentoffset = sceneLights.pointLights.size() * sizeof(GPU_DirectionalLight);
		memcpy(bufferWithOffset, sceneLights.directionalLights.data(), currentoffset);

		bufferWithOffset += currentoffset;
		currentoffset += sceneLights.pointLights.size() * sizeof(GPU_PointLight);
		memcpy(bufferWithOffset, sceneLights.pointLights.data(), currentoffset);

		bufferWithOffset += currentoffset;
		currentoffset += sceneLights.pointLights.size() * sizeof(GPU_PointLight);
		memcpy(bufferWithOffset, sceneLights.pointLights.data(), currentoffset);

		return &sceneData;
	}
}
