#include "DrnPCH.h"
#include "Pawn.h"

namespace Drn
{
	Pawn::Pawn() 
		: Actor()
		, m_AutoPossessPlayer(false)
	{
		m_RootComponent = std::make_unique<class SceneComponent>();
		m_RootComponent->SetComponentLabel( "RootComponent" );
		SetRootComponent(m_RootComponent.get());

		m_MeshComponent = std::make_unique<class StaticMeshComponent>();
		m_RootComponent->AttachSceneComponent(m_MeshComponent.get());

		AssetHandle<StaticMesh> SphereMesh( "Engine\\Content\\BasicShapes\\SM_Sphere.drn" );
		SphereMesh.Load();
		m_MeshComponent->SetMesh(SphereMesh);
	}

	Pawn::~Pawn()
	{
		
	}

	void Pawn::Tick( float DeltaTime )
	{
		Actor::Tick(DeltaTime);


	}

	EActorType Pawn::GetActorType()
	{
		return EActorType::Pawn;
	}

	void Pawn::Serialize( Archive& Ar )
	{
		Actor::Serialize(Ar);

		m_RootComponent->Serialize(Ar);
		m_MeshComponent->Serialize(Ar);

		if (Ar.IsLoading())
		{
			Ar >> m_AutoPossessPlayer;
		}

		else
		{
			Ar << m_AutoPossessPlayer;
		}
	}

}