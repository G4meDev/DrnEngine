#include "DrnPCH.h"
#include "Pawn.h"

#include "Runtime/Components/InputComponent.h"

#if WITH_EDITOR
#include <imgui.h>
#endif

namespace Drn
{
	Pawn::Pawn() 
		: Actor()
		, m_InputComponent(nullptr)
		, m_Controller(nullptr)
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
	{}

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

	void Pawn::PossessBy( Controller* InController )
	{
		UnPossess();

		m_Controller = InController;
		CreatePlayerInputComponent();
	}

	void Pawn::UnPossess()
	{
		DestroyPlayerInputComponent();

		if (m_Controller)
		{
			m_Controller = nullptr;
		}
	}

#if WITH_EDITOR
	bool Pawn::DrawDetailPanel()
	{
		bool Dirty = Actor::DrawDetailPanel();

		Dirty |= ImGui::Checkbox( "AutoPossessPlayer", &m_AutoPossessPlayer);

		return Dirty;
	}

	void Pawn::CreatePlayerInputComponent()
	{
		m_InputComponent = new InputComponent();
		m_InputComponent->SetComponentLabel("InputComponent");
		AddComponent(m_InputComponent);
		m_InputComponent->RegisterComponent(GetWorld());
	}

	void Pawn::DestroyPlayerInputComponent()
	{
		if (m_InputComponent)
		{
			m_InputComponent->DestroyComponent();
		}
	}

#endif
}