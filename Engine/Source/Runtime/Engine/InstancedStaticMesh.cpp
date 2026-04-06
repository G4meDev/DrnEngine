#include "DrnPCH.h"
#include "InstancedStaticMesh.h"

namespace Drn
{
	InstancedStaticMeshActor::InstancedStaticMeshActor()
	{
		m_InstancedStaticMeshComponenet = std::make_unique<InstancedStaticMeshComponent>();
		SetRootComponent(m_InstancedStaticMeshComponenet.get());

#if WITH_EDITOR
		m_InstancedStaticMeshComponenet->SetComponentLabel( "InstancedStaticMesh" );
#endif
	}

	InstancedStaticMeshActor::~InstancedStaticMeshActor()
	{

	}

	void InstancedStaticMeshActor::Serialize( Archive& Ar )
	{
		Actor::Serialize(Ar);

		m_InstancedStaticMeshComponenet->Serialize(Ar);
	}

	void InstancedStaticMeshActor::Tick( float DeltaTime )
	{
		Actor::Tick(DeltaTime);
	}

}  // namespace Drn