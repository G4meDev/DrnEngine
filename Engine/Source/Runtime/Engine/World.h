#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class World
	{
	public:

		World();
		~World();

		void AddStaticMeshCompponent(StaticMeshComponent* InStaticMesh);
		void RemoveStaticMeshCompponent(StaticMeshComponent* InStaticMesh);

	protected:

		// @TODO: promote to actor
		std::vector<StaticMeshComponent*> m_StaticMeshComponents;

		friend Scene;

	private:
	};
}