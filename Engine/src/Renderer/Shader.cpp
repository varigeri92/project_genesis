#include "gnspch.h"
#include "Shader.h"
#include "RenderSystem.h"
#include "Renderer.h"
#include "../AssetDatabase/AssetLoader.h"
#include "spirv_reflect/spirv_reflect.h"


gns::rendering::Shader::Shader(const std::string vertexShaderPath, const std::string fragmentShaderPath) :
	m_vertexShaderPath(vertexShaderPath),
	m_fragmentShaderPath(fragmentShaderPath)
{
	ReadAttributes();
	RenderSystem* renderSystem = SystemsAPI::GetSystem<RenderSystem>();
	renderSystem->CreatePipeline(this);
}

void gns::rendering::Shader::Dispose()
{
	RenderSystem* renderSystem = SystemsAPI::GetSystem<RenderSystem>();
	renderSystem->DisposeShader(this);
	Object::Dispose();
}

bool SpirvReflectExample(const void* spirv_code, size_t spirv_nbytes)
{
	bool sucess = false;
	// Generate reflection data for a shader
	SpvReflectShaderModule module = {};
	SpvReflectResult result = spvReflectCreateShaderModule(spirv_nbytes, spirv_code, &module);
	if(result != SPV_REFLECT_RESULT_SUCCESS)
	{
		LOG_ERROR("Failed to create Shader Module: " << result );
		return sucess;
	}
	// Enumerate and extract shader's input variables
	uint32_t var_count = 0;
	result = spvReflectEnumerateInputVariables(&module, &var_count, NULL);
	if (result != SPV_REFLECT_RESULT_SUCCESS)
	{
		sucess = false;
		LOG_ERROR("Failed to Enumerate Varialbles: " << result);

	}
	else
	{
		SpvReflectInterfaceVariable** input_vars =
			(SpvReflectInterfaceVariable**)malloc(var_count * sizeof(SpvReflectInterfaceVariable*));
		result = spvReflectEnumerateInputVariables(&module, &var_count, input_vars);
		if (result != SPV_REFLECT_RESULT_SUCCESS)
		{
			sucess = false;
			LOG_ERROR("Failed to Enumerate Varialbles: " << result);
		}
	}
	// Output variables, descriptor bindings, descriptor sets, and push constants
	// can be enumerated and extracted using a similar mechanism.

	// Destroy the reflection data when no longer required.
	spvReflectDestroyShaderModule(&module);
	return sucess;
}

void gns::rendering::Shader::ReadAttributes()
{
	//placeholder
	//TODO: Get the Attributes from the shader code, Save it, and also package the data with the game.
	std::vector<uint32_t> shader_code = gns::AssetLoader::LoadShader(m_fragmentShaderPath);
	 
	bool reflection_sucess = SpirvReflectExample(shader_code.data(), shader_code.size());
	if(reflection_sucess)
	{
		LOG_INFO("Shader ReflectionSucessfull!");
	}
	else
	{
		LOG_WARNING("Shader Reflection Failed!");
	}
	//set 0 is the global data

	fragmentShaderDataSize = 0;
	//set 2 is other per material data like color and other attributes
	m_fragmentShaderAttributes.emplace_back("color_albedo", ShaderAttributeType::Color4, 0, 2, 0);
	
	fragmentShaderDataSize += (sizeof(float) * 4);
	m_fragmentShaderAttributes.emplace_back("specular", 
		ShaderAttributeType::Float, fragmentShaderDataSize, 2, 0);
	fragmentShaderDataSize += (sizeof(float));
}
