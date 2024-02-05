#pragma once
#include <string>
#include <memory>

#include "gnsAPI.h"
#include "EnTT/entt.hpp"
#include "../Rendering/Device.h"
namespace gns
{
	class Entity;
	struct Scene
	{
		struct alignas(64) GPUSceneData {
			glm::vec4 fogColor; // w is for exponent
			glm::vec4 fogDistances; //x for min, y for max, zw unused.
			glm::vec4 ambientColor;
			glm::vec4 sunlightDirection; //w for sun power
			glm::vec4 sunlightColor;
		};

		std::string name;
		entt::registry registry;
		GPUSceneData sceneData;
		Scene(std::string name) : name{ name } {};

		GEN_API Entity CreateEntity(std::string name, Scene* scene);
		GEN_API void Destroy(Entity& entity);
	};
}

