#include "DrnPCH.h"
#include "NavigationSystem.h"

namespace Drn
{
	NavigationSystem::NavigationSystem()
	{
		
	}

	NavigationSystem::~NavigationSystem()
	{
		
	}

	void NavigationSystem::RegisterNavMeshComponent( NavMeshComponent* InNavMesh )
	{
		auto It = std::find(NavMeshes.begin(), NavMeshes.end(), InNavMesh);
		drn_check(It == NavMeshes.end());

		NavMeshes.push_back(InNavMesh);
	}

	void NavigationSystem::UnregisterNavMeshComponent( NavMeshComponent* InNavMesh )
	{
		auto It = std::find(NavMeshes.begin(), NavMeshes.end(), InNavMesh);
		drn_check(It != NavMeshes.end());

		NavMeshes.erase(It);
	}

	NavMeshComponent* NavigationSystem::GetNavMesh( const std::string& Name )
	{
		auto It = std::find_if(NavMeshes.begin(), NavMeshes.end(), [&Name](NavMeshComponent* NavMesh){ return NavMesh->Name == Name; });
		return It == NavMeshes.end() ? nullptr : *It;
	}

}  // namespace Drn