#include "gnspch.h"
#include "Shader.h"
#include "RenderSystem.h"
#include "Renderer.h"

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

void gns::rendering::Shader::ReadAttributes()
{
	//placeholder
	//TODO: Get the Attributes from the shader code, Save it, and also package the data with the game.

	//set 0 is the global data

	fragmentShaderDataSize = 0;
	//set 2 is other per material data like color and other attributes
	m_fragmentShaderAttributes.emplace_back("color_albedo", ShaderAttributeType::Color4, 0, 2, 0);
	
	fragmentShaderDataSize += (sizeof(float) * 4);
	m_fragmentShaderAttributes.emplace_back("specular", 
		ShaderAttributeType::Float, fragmentShaderDataSize, 2, 0);
	/*
	 
	//set 1 is for textures
	m_fragmentShaderAttributes.emplace_back("tex_albedo", ShaderAttributeType::Texture, 0, 1, 0);
	m_fragmentShaderAttributes.emplace_back("tex_normal", ShaderAttributeType::Texture, 0, 1, 0);

	m_fragmentShaderAttributes.emplace_back("tex_metallic", ShaderAttributeType::Texture, 0, 1, 0);
	m_fragmentShaderAttributes.emplace_back("color_specular", ShaderAttributeType::Color4, fragmentShaderDataSize, 2, 0);
	fragmentShaderDataSize += (sizeof(float) * 4);

	m_fragmentShaderAttributes.emplace_back("tex_roughness", ShaderAttributeType::Texture, 0, 1, 0);
	m_fragmentShaderAttributes.emplace_back("roughness", ShaderAttributeType::Float, fragmentShaderDataSize, 2, 0);
	fragmentShaderDataSize += sizeof(float);

	m_fragmentShaderAttributes.emplace_back("tex_ao",ShaderAttributeType::Texture, 0, 1, 0 );
	*/
}
