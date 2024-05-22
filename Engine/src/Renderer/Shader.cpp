#include "gnspch.h"
#include "Shader.h"
#include "RenderSystem.h"
#include "Renderer.h"

gns::rendering::Shader::Shader(const std::string vertexShaderPath, const std::string fragmentShaderPath) :
	m_vertexShaderPath(vertexShaderPath),
	m_fragmentShaderPath(fragmentShaderPath)
{
	ReadAttributes();
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
	
	//set 1 is for textures
	m_fragmentShaderAttributes["tex_color"]		= { /*type*/ShaderAttributeType::Texture, /*set*/1, /*binding*/0 };
	m_fragmentShaderAttributes["tex_normal"]	= { /*type*/ShaderAttributeType::Texture, /*set*/1, /*binding*/1 };
	m_fragmentShaderAttributes["tex_metallic"]	= { /*type*/ShaderAttributeType::Texture, /*set*/1, /*binding*/2 };
	m_fragmentShaderAttributes["tex_roughness"] = { /*type*/ShaderAttributeType::Texture, /*set*/1, /*binding*/3 };
	m_fragmentShaderAttributes["tex_ao"]		= { /*type*/ShaderAttributeType::Texture, /*set*/1, /*binding*/4 };

	//set 2 is other per material data like color and other attributes
	m_fragmentShaderAttributes["albedoColor"]	= {	/*type*/ShaderAttributeType::Color4,	/*set*/2, /*binding*/0 };
	m_fragmentShaderAttributes["specularColor"] = {	/*type*/ShaderAttributeType::Color4,	/*set*/2, /*binding*/0 };
	m_fragmentShaderAttributes["EmissionColor"] = {	/*type*/ShaderAttributeType::HdrColor4,	/*set*/2, /*binding*/0 };
	m_fragmentShaderAttributes["Roughness"]		= {	/*type*/ShaderAttributeType::Float,		/*set*/2, /*binding*/0 };
}
