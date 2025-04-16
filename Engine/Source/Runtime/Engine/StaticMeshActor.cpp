#include "DrnPCH.h"
#include "StaticMeshActor.h"

namespace Drn
{
	StaticMeshActor::StaticMeshActor()
		: Actor()
	{
		m_MeshComponenet = std::make_unique<StaticMeshComponent>();
		GetRoot()->AttachSceneComponent(m_MeshComponenet.get());

#if WITH_EDITOR
		m_MeshComponenet->SetComponentLabel( "Mesh" );
#endif
	}

	StaticMeshActor::~StaticMeshActor()
	{
		
	}

	void StaticMeshActor::Tick( float DeltaTime )
	{
		Actor::Tick(DeltaTime);

		//std::cout << "ticking \n";
		//
		//XMVECTOR NewLocation = GetActorLocation() + XMVectorSet(0, -9 * DeltaTime, 0, 0);
		//SetActorLocation(NewLocation);
	}

	void StaticMeshActor::Serialize( Archive& Ar )
	{
		Actor::Serialize(Ar);

		if (Ar.IsLoading())
		{
			m_MeshComponenet->Serialize(Ar);
			
		}

#if WITH_EDITOR

		else
		{
			m_MeshComponenet->Serialize(Ar);

		}

#endif
	}

}