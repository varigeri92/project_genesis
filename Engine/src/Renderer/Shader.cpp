#include "gnspch.h"
#include "Shader.h"
#include "RenderSystem.h"
#include "Renderer.h"
#include "../AssetDatabase/AssetLoader.h"
#include "spirv_reflect/spirv_reflect.h"
#include "spirv_cross/spirv_cross.hpp"


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
	std::vector<uint32_t> spirv = gns::AssetLoader::LoadShader(m_fragmentShaderPath);
	spirv_cross::Compiler comp(std::move(spirv));
	spirv_cross::ShaderResources res = comp.get_shader_resources();
	LOG_INFO("Shader Data of: " << m_fragmentShaderPath);
	LOG_INFO("\t Uniforms:");
	for (size_t i = 0; i < res.uniform_buffers.size(); i++)
	{
		auto& type = comp.get_type(res.uniform_buffers[i].type_id);
		LOG_INFO("\t\t" << res.uniform_buffers[i].name << " " << res.uniform_buffers[i].type_id << " " << res.uniform_buffers[i].id << " " << res.uniform_buffers[i].base_type_id);
		auto member_types = type.member_types;
		auto member_names = comp.get_type(res.uniform_buffers[i].type_id).member_name_cache;
		for (size_t j = 0; j < member_types.size(); j++) {
			auto& name = comp.get_member_name(type.self, j);
			auto& member_type = comp.get_type(member_types[i]);
			size_t offset = comp.type_struct_member_offset(type, j);
			LOG_INFO("\t\t\t" << member_types[j] << " " << name << ", offset: " << offset);
		}
	}

	LOG_INFO("\t Sampled Images:");
	for (size_t i = 0; i < res.sampled_images.size(); i++)
	{
		LOG_INFO("\t\t" << res.sampled_images[i].name << " " << res.sampled_images[i].type_id << " " << res.sampled_images[i].id << " " << res.sampled_images[i].base_type_id);
	}

	LOG_INFO("\t Storage buffers:");
	for (size_t i = 0; i < res.gl_plain_uniforms.size(); i++)
	{
		LOG_INFO("\t\t" << res.gl_plain_uniforms[i].name << " " << res.gl_plain_uniforms[i].type_id << " " << res.gl_plain_uniforms[i].id << " " << res.gl_plain_uniforms[i].base_type_id);
	}


	fragmentShaderDataSize = 0;
	//set 2 is other per material data like color and other attributes
	m_fragmentShaderAttributes.emplace_back("color_albedo", ShaderAttributeType::Color4, 0, 2, 0);
	
	fragmentShaderDataSize += (sizeof(float) * 4);
	m_fragmentShaderAttributes.emplace_back("specular", 
		ShaderAttributeType::Float, fragmentShaderDataSize, 2, 0);
	fragmentShaderDataSize += (sizeof(float));
}
