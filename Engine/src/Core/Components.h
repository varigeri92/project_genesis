#pragma once
#include <memory>
#include <string>
#include <vector>

#include "glm/glm.hpp"

namespace gns
{
	struct Mesh;
}

namespace gns::core
{
	struct EntityComponent
	{
		std::string name;
		bool isEnabled;
		bool isStatic;
	};

	struct Position
	{
		glm::vec3 vector;
	};

	struct Rotation
	{
		glm::vec3 vector;
	};

	struct Scale
	{
		glm::vec3 vector;
	};

	struct Transform
	{
		glm::mat4 matrix;
	};

	struct MeshComponent
	{
		std::shared_ptr<Mesh> mesh;
	};
}
