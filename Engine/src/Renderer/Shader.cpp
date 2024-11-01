﻿#include "gnspch.h"
#include "Shader.h"
#include "RenderSystem.h"
#include "Renderer.h"
#include "../AssetDatabase/AssetLoader.h"
#include "spirv_reflect/spirv_reflect.h"
#include "spirv_cross/spirv_cross.hpp"


const static std::string SceneAttributes_UBOName = "SceneData";

gns::rendering::Shader::Shader(const std::string vertexShaderPath, const std::string fragmentShaderPath) :
	m_vertexShaderPath(vertexShaderPath),
	m_fragmentShaderPath(fragmentShaderPath)
{
	pipeline = VK_NULL_HANDLE;
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

void gns::rendering::Shader::ReadAttributes()
{

	std::vector<uint32_t> spirv = gns::AssetLoader::LoadShader(m_fragmentShaderPath);
	spirv_cross::Compiler comp(std::move(spirv));
	spirv_cross::ShaderResources res = comp.get_shader_resources();
	LOG_INFO("Shader Data of: " << m_fragmentShaderPath);
	LOG_INFO("\t Uniforms:");
	for (size_t i = 0; i < res.uniform_buffers.size(); i++)
	{
		auto& type = comp.get_type(res.uniform_buffers[i].type_id);
		uint32_t set = comp.get_decoration(res.uniform_buffers[i].id, spv::DecorationDescriptorSet);
		uint32_t binding = comp.get_decoration(res.uniform_buffers[i].id, spv::DecorationBinding);
		LOG_INFO("\t\t" << res.uniform_buffers[i].name << " " << res.uniform_buffers[i].type_id << " " << res.uniform_buffers[i].id << " " << res.uniform_buffers[i].base_type_id);
		LOG_INFO("\t\t" << res.uniform_buffers[i].name << "(set = "<< set << ", binding = " << binding <<")");
		auto member_types = type.member_types;
		auto member_names = comp.get_type(res.uniform_buffers[i].type_id).member_name_cache;
		if (res.uniform_buffers[i].name == "SceneData")
			continue;
		fragmentShaderDataSize = 0;
		for (size_t j = 0; j < member_types.size(); j++) {
			auto name = comp.get_member_name(type.self, j);
			auto member_type = comp.get_type(member_types[i]);
			size_t member_size = comp.get_declared_struct_member_size(type, j);
			size_t offset = comp.type_struct_member_offset(type, j);
			LOG_INFO("\t\t\t" << member_types[j] << " " << name << ", offset: " << offset << " size: " << member_size );
			AddAttribute(name, offset, member_types[j], set, binding);
			fragmentShaderDataSize += member_size;
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
}

gns::rendering::Shader::ShaderAttributeType gns::rendering::Shader::GetAttributeType(std::string& name, uint32_t typeID)
{
	ShaderAttributeType type = attributeTypeMap[typeID];
	if(type == ShaderAttributeType::Float4)
	{
		if (name.starts_with("c_"))
		{
			type = ShaderAttributeType::Color4;
		}
	}
	else if (type == ShaderAttributeType::Float3)
	{
		if(name.starts_with("c_"))
		{
			type = ShaderAttributeType::Color;
		}
	}
	return type;
}


void gns::rendering::Shader::AddAttribute(std::string& name, size_t offset, uint32_t type_ID, uint32_t set, uint32_t binding)
{
	m_fragmentShaderAttributes.emplace_back(name, GetAttributeType(name, type_ID), offset, set, binding);
}
