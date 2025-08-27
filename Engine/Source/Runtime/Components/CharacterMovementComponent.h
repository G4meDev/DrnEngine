#pragma once

#include "ForwardTypes.h"
#include "Runtime/Engine/Component.h"
#include "Runtime/Physic/PhysicUserData.h"

namespace Drn
{
	class CharacterMovementComponent : public Component
	{
	public:
		CharacterMovementComponent();
		virtual ~CharacterMovementComponent();

		void Tick( float DeltaTime ) override;

		EComponentType GetComponentType() override { return EComponentType::CharacterMovementComponent; };
		inline static EComponentType GetComponentTypeStatic() { return EComponentType::CharacterMovementComponent; };

		void Serialize( Archive& Ar ) override;

		void RegisterComponent( World* InOwningWorld ) override;
		void UnRegisterComponent() override;

		void SetMovementInput(const Vector& Input);

		inline Vector GetPosition() const { return m_Controller ? Pd2Vector(m_Controller->getFootPosition()) : GetOwningActor()->GetActorLocation(); }

		void SendPhysicTranform(const Transform& InTransform);

		Vector CalculateCapsulePosition() const;
		float CalculateRealHeight() const;

		void SetHalfHeight(float HalfHeight);
		void SetRadius(float Radius);

		inline float GetHalfHeight() const { return m_HalfHeight; }
		inline float GetRadius() const { return m_Radius; }

		PhysicUserData& GetUserData()
		{
			PhysicUserData::Set<CharacterMovementComponent>((void*)&UserData, const_cast<CharacterMovementComponent*>(this));
			return UserData;
		}

#if WITH_EDITOR
		void DrawDetailPanel( float DeltaTime ) override;
#endif

	protected:

		float m_HalfHeight;
		float m_Radius;

		physx::PxController* m_Controller;
		Vector m_MovementInput;

		PhysicUserData UserData;
	};
}