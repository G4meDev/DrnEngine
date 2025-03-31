#include "DrnPCH.h"
#include "StaticMeshComponent.h"

namespace Drn
{
	StaticMeshComponent::StaticMeshComponent()
		: PrimitiveComponent()
	{
		
	}

	StaticMeshComponent::~StaticMeshComponent()
	{
		std::cout << "Remove";
	}

	void StaticMeshComponent::Tick( float DeltaTime )
	{
		PrimitiveComponent::Tick(DeltaTime);
	}

	void StaticMeshComponent::SetMesh( const AssetHandle<StaticMesh>& InHandle )
	{
		Mesh = InHandle;
	}


}