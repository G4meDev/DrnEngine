#include "DrnPCH.h"
#include "InstancedStaticMesh.h"

namespace Drn
{
	InstancedStaticMeshActor::InstancedStaticMeshActor()
	{
		m_InstancedStaticMeshComponenet = std::make_unique<InstancedStaticMeshComponent>();
		SetRootComponent(m_InstancedStaticMeshComponenet.get());
		//GetRoot()->AttachSceneComponent(m_MeshComponenet.get());

#if WITH_EDITOR
		m_InstancedStaticMeshComponenet->SetComponentLabel( "InstancedStaticMesh" );
#endif
	}

	InstancedStaticMeshActor::~InstancedStaticMeshActor()
	{

	}

	void InstancedStaticMeshActor::Tick( float DeltaTime )
	{
		Actor::Tick(DeltaTime);
	}

//	void InstancedStaticMeshActor::Serialize( Archive& Ar )
//	{
//		Actor::Serialize(Ar);
//
//		if (Ar.IsLoading())
//		{
//			m_MeshComponenet->Serialize(Ar);
//			
//		}
//
//#if WITH_EDITOR
//
//		else
//		{
//			m_MeshComponenet->Serialize(Ar);
//
//		}
//
//#endif
//	}

}  // namespace Drn