#version 460
#include "include.frag.glsl"

//Scene
layout(set = 0, binding = 1) uniform  SceneData{
	vec4 ambientColor;
	vec4 sunlightDirection; //w for sun power
	vec4 sunlightColor;
	vec3 viewPos;

	AmbientLight ambient;
	DirectionalLight directionalLights[MAX_LIGHTS];
	PointLight pointlights[MAX_LIGHTS];
} sceneData;

//materialData
layout(set = 1, binding = 0) uniform MaterialData{
	vec4 albedo;
	float specular;
} materialData;
//textures
layout(set = 1, binding = 1) uniform sampler2D tex_color;
layout(set = 1, binding = 2) uniform sampler2D tex_normal;
layout(set = 1, binding = 3) uniform sampler2D tex_metallic;
layout(set = 1, binding = 4) uniform sampler2D tex_roughness;
layout(set = 1, binding = 5) uniform sampler2D tex_ao;

//output write
layout (location = 0) out vec4 outFragColor;

void main()
{
	float ambientStrength = sceneData.ambientColor.w;
	vec3 ambient = ambientStrength * sceneData.ambientColor.xyz;
	
	float diff = (dot(inNormal, sceneData.sunlightDirection.xyz) + 1) * 0.5;
	vec3 diffuse = diff * sceneData.sunlightColor.xyz;

	vec3 viewDir = normalize(sceneData.viewPos - inPos);
	vec3 reflectDir = reflect(-sceneData.sunlightDirection.xyz, inNormal);

	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	vec3 specular = materialData.specular * spec * sceneData.sunlightColor.xyz;
	
	vec3 result = (ambient + diffuse + specular) * materialData.albedo.xyz;
	outFragColor = vec4(result, 1.0);
	
	/*
	float diff = max((dot(inNormal, sceneData.sunlightDirection.xyz) + 1) * 0.5, 0.0);
	diff += sceneData.ambientColor.w;
	vec3 diffuse = sceneData.sunlightColor.xyz * diff * sceneData.sunlightColor.w;
	vec3 color = texture(tex_color, inTexCoord).xyz * materialData.albedo.xyz;
	outFragColor = vec4((color * diffuse) * (sceneData.ambientColor.xyz),1.0f);
	*/
}