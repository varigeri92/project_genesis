#version 460

const int MAX_LIGHTS = 100;

struct DirectionalLight{
    vec3 direction;
    float intensity;
    vec4 color;
};
struct AmbientLight{
    float intensity;
    vec4 color;
};
struct PointLight{
    vec3 position;
    vec4 color;
    float intensity;
    float range;
};

struct SpotLight{
    vec3 position;
    vec3 direction;
    vec4 color;
    float intensity;
    float range;
    float angle;
};
//shader input
layout (location = 0) in vec3 inColor;
layout (location = 1) in vec2 inTexCoord;
layout (location = 2) in vec3 inNormal;
layout (location = 3) in vec3 inPos;

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
	vec4 c_albedo;
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

	float diff = max(dot(inNormal, sceneData.sunlightDirection.xyz), 0.0);
    vec3 diffuse = diff * sceneData.sunlightColor.xyz;
    vec3 result = (ambient + diffuse) * materialData.c_albedo.xyz;
	outFragColor = vec4(result, 1.0);
}