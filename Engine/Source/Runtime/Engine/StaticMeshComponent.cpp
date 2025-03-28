#include "DrnPCH.h"
#include "StaticMeshComponent.h"

namespace Drn
{
	void StaticMeshComponent::SetMesh( const AssetHandle<StaticMesh>& InHandle )
	{
		Mesh = InHandle;
	}


}