//glsl version 4.6
#version 460
//shader input
layout (location = 0) in vec3 inColor;
layout (location = 1) in vec2 inTexCoord;
layout (location = 2) in vec3 inNormal;
//textures
layout(set = 1, binding = 0) uniform sampler2D tex_color;
layout(set = 1, binding = 1) uniform sampler2D tex_normal;
layout(set = 1, binding = 2) uniform sampler2D tex_metallic;
layout(set = 1, binding = 3) uniform sampler2D tex_roughness;
layout(set = 1, binding = 4) uniform sampler2D tex_ao;

//output write
layout (location = 0) out vec4 outFragColor;

layout(set = 0, binding = 1) uniform  SceneData{
    vec4 fogColor; // w is for exponent
	vec4 fogDistances; //x for min, y for max, zw unused.
	vec4 ambientColor;
	vec4 sunlightDirection; //w for sun power
	vec4 sunlightColor;
} sceneData;


void main()
{
	
	float diff = max((dot(inNormal, sceneData.sunlightDirection.xyz) + 1) * 0.5, 0.0);
	diff += sceneData.ambientColor.w;
	vec3 diffuse = sceneData.sunlightColor.xyz * diff * sceneData.sunlightColor.w;
	vec3 unlitSolidColor = vec3(0.9, 0.7, 0.5);
	
	vec3 color = texture(tex_color, inTexCoord).xyz;
	outFragColor = vec4((color * diffuse) * (sceneData.ambientColor.xyz),1.0f);
}