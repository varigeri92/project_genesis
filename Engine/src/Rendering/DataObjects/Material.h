#pragma once
#include <memory>
#include <string>
#include <glm/gtx/transform.hpp>
#include "vulkan/vulkan.h"
#include "DisposableVkObject.h"

namespace gns::rendering
{
	class Mesh;
	struct Shader
	{
		Shader(std::string vertexShaderPath, std::string fragmentShaderPath) :
			vertexShaderPath(vertexShaderPath),
			fragmentShaderPath(fragmentShaderPath) {};


		std::string vertexShaderPath;
		std::string fragmentShaderPath;
	};
	struct Material : public Disposeable {
		Material(std::shared_ptr<Shader> shader) :shader(shader){};

		VkPipeline pipeline;
		VkPipelineLayout pipelineLayout;
		std::shared_ptr<Shader> shader;

		void Dispose(Device* device) override;
	};
}

