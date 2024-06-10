#pragma once

struct Color
{
	float r;
	float g;
	float b;
	float a;

	operator glm::vec4() { return { r,g,b,a }; }
	operator glm::vec4&() { return {reinterpret_cast<glm::vec4&>(*this)}; }
	operator float* () { return &r; }
};

struct Color3
{
	float r;
	float g;
	float b;

	operator glm::vec3() { return { r,g,b }; }
	operator glm::vec3& () { return { reinterpret_cast<glm::vec3&>(*this) }; }
	operator float* () { return &r; }
};