#include "DrnPCH.h"
#include "SkeletalMeshActor.h"

namespace Drn
{
	SkeletalMeshActor::SkeletalMeshActor()
		: Actor()
	{
		m_MeshComponenet = std::make_unique<SkeletalMeshComponent>();
		SetRootComponent(m_MeshComponenet.get());
		//GetRoot()->AttachSceneComponent(m_MeshComponenet.get());

#if WITH_EDITOR
		m_MeshComponenet->SetComponentLabel( "Mesh" );
#endif
	}

	SkeletalMeshActor::~SkeletalMeshActor()
	{
	}

	void SkeletalMeshActor::Tick( float DeltaTime )
	{
		Actor::Tick(DeltaTime);

		//std::cout << "ticking \n";
		//
		//XMVECTOR NewLocation = GetActorLocation() + XMVectorSet(0, -9 * DeltaTime, 0, 0);
		//SetActorLocation(NewLocation);
	}

	void SkeletalMeshActor::Serialize( Archive& Ar )
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