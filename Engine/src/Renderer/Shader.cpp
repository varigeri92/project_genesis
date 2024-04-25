#include "gnspch.h"
#include "Shader.h"
#include "RenderSystem.h"
#include "Renderer.h"

gns::rendering::Shader::Shader(const std::string vertexShaderPath, const std::string fragmentShaderPath) :
	m_vertexShaderPath(vertexShaderPath),
	m_fragmentShaderPath(fragmentShaderPath)
{
}

gns::rendering::Shader::~Shader()
{
	RenderSystem* renderSystem = SystemsAPI::GetSystem<RenderSystem>();
	renderSystem->DisposeShader(this);
}
