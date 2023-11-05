#pragma once
#include <vulkan/vulkan.h>
#include <string>
#include <memory>
#include <vector>
#include <glm/glm.hpp>

#include "Log.h"
#include "Texture.h"
#include "../AssetDatabase/Guid.h"

namespace gns::rendering
{

	typedef enum PipelineFeatures
	{
		PIPELINE_FEATURE_WRITE_DEPTH = 0x00000001,
		PIPELINE_FEATURE_ALPHA_BLEND = 0x00000002,
	} PipelineFeatures;

	struct ShaderUBO_Descriptor
	{

	};

	struct PushConstant
	{
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 projection;
	};

	struct Shader
	{
		core::guid guid;
		std::string name;
		std::vector<std::string> Keywords;
		PipelineFeatures featureFlags;
		std::string vertex_path;
		std::string fragment_path;

		Shader(std::string path, std::string name) : name{ name }, featureFlags { PIPELINE_FEATURE_WRITE_DEPTH }
		{
			guid = core::Guid::GetNewGuid();
			vertex_path = path + ".vert.spv";
			fragment_path = path + ".frag.spv";
			LOG_INFO("ShaderCreated: " << name << " / guid: " << guid <<"\n\t" << vertex_path << "\n\t" << fragment_path);
		}
	};
	struct Material
	{
		core::guid guid;
		std::string name;
		std::shared_ptr<Shader> shader;
		std::shared_ptr<Texture> texture;

		Material(std::string name, std::string shaderName) : name {name}
		{
			guid = 100;//core::Guid::GetNewGuid();
			shader = std::make_shared<Shader>(R"(D:\GenesisEngine\Engine\Assets\Shaders\default)", shaderName);
			LOG_INFO("Material Created: " << name << " / guid: " << guid);

		}

		Material(std::string name, std::string shaderName, std::shared_ptr<Texture> texture) : name{ name }, texture{ texture }
		{
			guid = core::Guid::GetNewGuid();
			shader = std::make_shared<Shader>(R"(D:\GenesisEngine\Engine\Assets\Shaders\default)", shaderName);
			LOG_INFO("Material Created: " << name << " / guid: " << guid);
			
		}
	};
}

