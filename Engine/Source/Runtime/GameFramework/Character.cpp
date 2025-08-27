#include "DrnPCH.h"
#include "Character.h"
#include "Runtime/Components/InputComponent.h"
#include "Runtime/Components/CharacterMovementComponent.h"

namespace Drn
{
	Character::Character()
		: Pawn()
	{
		m_MovementComponent = new CharacterMovementComponent();
		m_MovementComponent->SetComponentLabel("MovementComponent");
		AddComponent(m_MovementComponent);

		GetRoot()->OnTransformUpdateDel.Add(this, &Character::OnRootTransformUpdate);
	}

	Character::~Character()
	{
		
	}

	void Character::Tick( float DeltaTime )
	{
		Pawn::Tick(DeltaTime);

		m_MovementComponent->SetMovementInput(m_MovementInput * 2);
		m_MovementInput = Vector::ZeroVector;
	}

	void Character::Serialize( Archive& Ar )
	{
		Pawn::Serialize(Ar);

		if (Ar.IsLoading())
		{
			m_MovementComponent->Serialize(Ar);
		}

		else
		{
			m_MovementComponent->Serialize(Ar);

		}
	}

	void Character::SetupPlayerInputComponent( class InputComponent* PlayerInputComponent )
	{
		PlayerInputComponent->AddAxis(1, 1.0f, 1.0f, this, &Character::OnMoveForward);
		PlayerInputComponent->AddAxisMapping(1, gainput::KeyW, 1);
		PlayerInputComponent->AddAxisMapping(1, gainput::KeyS, -1);

		PlayerInputComponent->AddAxis(2, 1.0f, 1.0f, this, &Character::OnMoveRight);
		PlayerInputComponent->AddAxisMapping(2, gainput::KeyD, 1);
		PlayerInputComponent->AddAxisMapping(2, gainput::KeyA, -1);

		PlayerInputComponent->AddAnalog(3, this, &Character::OnLookRight);
		PlayerInputComponent->AddAnalogMapping(3, gainput::MouseAxisX, 1);

		PlayerInputComponent->AddAnalog(4, this, &Character::OnLookUp);
		PlayerInputComponent->AddAnalogMapping(4, gainput::MouseAxisY, 1);

	}

	void Character::GetActorEyesViewPoint( Vector& OutLocation, Quat& OutRotation ) const
	{
		OutLocation = GetActorLocation() + Vector(0, 28, -20);
		OutRotation = Matrix::MakeFromZ( GetActorLocation() - OutLocation ).Rotation();
	}

	void Character::OnMoveForward( float Value )
	{
		//SetActorLocation( GetActorLocation() + GetActorForwardVector() * Value * 100 * Time::GetApplicationDeltaTime() );
		m_MovementInput.SetZ(Value * 0.1f);
	}

	void Character::OnMoveRight( float Value )
	{
		m_MovementInput.SetX(Value * 0.1f);
		//SetActorLocation( GetActorLocation() + GetActorRightVector() * Value * 100 * Time::GetApplicationDeltaTime() );
	}

	void Character::OnLookUp( float Value )
	{
		
	}

	void Character::OnLookRight( float Value )
	{
		
	}

	void Character::OnRootTransformUpdate( SceneComponent* Comp, bool bSkipPhysic )
	{
		if (!bSkipPhysic)
		{
			m_MovementComponent->SendPhysicTranform(GetActorTransform());
		}
	}

#if WITH_EDITOR
	bool Character::DrawDetailPanel()
	{
		bool Dirty = Pawn::DrawDetailPanel();

		return Dirty;
	}
#endif
}