#version 450
layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec3 vColor;
layout (location = 3) in vec2 vTexCoord;

layout (location = 0) out vec3 outColor;
layout (location = 1) out vec2 texCoord;
layout (location = 2) out vec3 normal;

layout(set = 0, binding = 0) uniform  CameraBuffer{
	mat4 view;
	mat4 proj;
	mat4 viewproj;
} cameraData;

//push constants block
layout( push_constant ) uniform constants
{
	mat4 model_matrix;
	mat4 camera_matrix;
} PushConstants;

void main()
{
	mat4 transformMatrix = (cameraData.viewproj * PushConstants.model_matrix);
	gl_Position = transformMatrix * vec4(vPosition, 1.0f);
	outColor = vColor;
	texCoord = vTexCoord;
	//normal = vNormal;
	normal = mat3(transpose(inverse(PushConstants.model_matrix))) * vNormal; 
}
