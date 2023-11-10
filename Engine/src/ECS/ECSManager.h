#pragma once
#include "../Core/Scene.h"
#include "Entity.h"

namespace gns 
{
	class ECSManager
	{
	public:
		static Entity CreateEntity(std::string name, std::shared_ptr<Scene> scene);
		static Entity CreateEntity(std::shared_ptr<Scene> scene);
		static void Destroy(Entity& entity, std::shared_ptr<Scene> scene);
	};
}

