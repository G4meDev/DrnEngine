#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class NavigationSystem
	{
	public:
		NavigationSystem();
		~NavigationSystem();

		void RegisterNavMeshComponent(NavMeshComponent* InNavMesh);
		void UnregisterNavMeshComponent(NavMeshComponent* InNavMesh);
		NavMeshComponent* GetNavMesh(const std::string& Name);

	protected:
		std::vector<NavMeshComponent*> NavMeshes;
	};
}