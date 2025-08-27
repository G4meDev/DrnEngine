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
		m_Controller = InController;

		//TODO: check for player controller
		CreatePlayerInputComponent();
	}

	void Pawn::UnPossess()
	{
		DestroyPlayerInputComponent();

		m_Controller = nullptr;
	}

	void Pawn::SetupPlayerInputComponent( class InputComponent* PlayerInputComponent )
	{
		PlayerInputComponent->AddAxis(1, 1.0f, 1.0f, this, &Pawn::OnMoveForward);
		PlayerInputComponent->AddAxisMapping(1, gainput::KeyW, 1);
		PlayerInputComponent->AddAxisMapping(1, gainput::KeyS, -1);

		PlayerInputComponent->AddAxis(2, 1.0f, 1.0f, this, &Pawn::OnMoveRight);
		PlayerInputComponent->AddAxisMapping(2, gainput::KeyD, 1);
		PlayerInputComponent->AddAxisMapping(2, gainput::KeyA, -1);

		PlayerInputComponent->AddAnalog(3, this, &Pawn::OnLookRight);
		PlayerInputComponent->AddAnalogMapping(3, gainput::MouseAxisX, 1);

		PlayerInputComponent->AddAnalog(4, this, &Pawn::OnLookUp);
		PlayerInputComponent->AddAnalogMapping(4, gainput::MouseAxisY, 1);
	}

	void Pawn::CreatePlayerInputComponent()
	{
		m_InputComponent = new InputComponent();
		m_InputComponent->SetComponentLabel("InputComponent");
		AddComponent(m_InputComponent);
		m_InputComponent->RegisterComponent(GetWorld());
		SetupPlayerInputComponent(m_InputComponent);
	}

	void Pawn::DestroyPlayerInputComponent()
	{
		if (m_InputComponent)
		{
			m_InputComponent->DestroyComponent();
		}
	}

	void Pawn::OnMoveForward( float Value )
	{
		SetActorLocation( GetActorLocation() + GetActorForwardVector() * Value * 100 * Time::GetApplicationDeltaTime() );
	}

	void Pawn::OnMoveRight( float Value )
	{
		SetActorLocation( GetActorLocation() + GetActorRightVector() * Value * 100 * Time::GetApplicationDeltaTime() );
	}

	void Pawn::OnLookUp( float Value )
	{
		SetActorRotation( Quat( GetActorRightVector(), Value * -0.005f) * GetActorRotation() );
	}

	void Pawn::OnLookRight( float Value )
	{
		SetActorRotation( Quat( Vector::UpVector, Value * -0.005f) * GetActorRotation());
	}

#if WITH_EDITOR
	bool Pawn::DrawDetailPanel()
	{
		bool Dirty = Actor::DrawDetailPanel();

		Dirty |= ImGui::Checkbox( "AutoPossessPlayer", &m_AutoPossessPlayer);

		return Dirty;
	}
#endif
}