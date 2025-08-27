#pragma once

#include "ForwardTypes.h"
#include "Runtime/GameFramework/Pawn.h"

namespace Drn
{
	class Character : public Pawn
	{
	public:
		Character();
		virtual ~Character();

		void Tick( float DeltaTime ) override;
		EActorType GetActorType() override { return EActorType::Character; };
		inline static EActorType GetActorTypeStatic() { return EActorType::Character; };
		void Serialize( Archive& Ar ) override;
		void SetupPlayerInputComponent( class InputComponent* PlayerInputComponent ) override;
		void GetActorEyesViewPoint( Vector& OutLocation, Quat& OutRotation ) const override;

		void OnMoveForward( float Value );
		void OnMoveRight(float Value);

		void OnLookUp(float Value);
		void OnLookRight(float Value);

		void OnRootTransformUpdate(SceneComponent* Comp, bool bSkipPhysic);

		inline class CharacterMovementComponent* GetCharacterMovementComponent() const { return m_MovementComponent; }

#if WITH_EDITOR
		bool DrawDetailPanel() override;
#endif
	protected:

		// TODO: remove
		Vector m_MovementInput = Vector::ZeroVector;
		class CharacterMovementComponent* m_MovementComponent;
	};
}