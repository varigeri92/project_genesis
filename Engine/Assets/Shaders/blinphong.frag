//glsl version 4.5
#version 460

//shader input
layout (location = 0) in vec3 inColor;
layout (location = 1) in vec2 inTexCoord;
layout (location = 2) in vec3 inNormal;
//textures
layout(set = 1, binding = 0) uniform sampler2D tex1;
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
	float diff = max(dot(inNormal, sceneData.sunlightDirection.xyz), 0.0);
	vec3 diffuse = sceneData.sunlightColor.xyz * diff * sceneData.sunlightColor.w;
	vec3 color = texture(tex1,inTexCoord).xyz;
	outFragColor = vec4( (color * diffuse) + (sceneData.ambientColor.xyz) * sceneData.ambientColor.w ,1.0f);
}