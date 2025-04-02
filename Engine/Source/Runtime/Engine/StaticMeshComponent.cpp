#include "DrnPCH.h"
#include "StaticMeshComponent.h"

#if WITH_EDITOR

#include <imgui.h>

#endif

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


	void StaticMeshComponent::DrawDetailPanel( float DeltaTime )
	{
		SceneComponent::DrawDetailPanel(DeltaTime);

		ImGui::Separator();


	}

}