#pragma once
#include <glm/glm.hpp>
namespace gns::rendering
{
	struct DirectionalLight
	{
		glm::vec4 color_intensity;
		glm::vec4 direction;
	};

	struct AmbientLight
	{
		glm::vec4 color_intensity;
	};
}
