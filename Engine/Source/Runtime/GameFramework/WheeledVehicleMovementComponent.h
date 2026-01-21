#pragma once

#include "ForwardTypes.h"
#include "Runtime/Engine/Component.h"

namespace Drn
{
	class WheeledVehicleMovementComponent: public Component
	{
	public:
		WheeledVehicleMovementComponent();
		virtual ~WheeledVehicleMovementComponent();

		void Tick( float DeltaTime ) override;

		EComponentType GetComponentType() override { return EComponentType::WheeledVehicleMovementComponent; };
		inline static EComponentType GetComponentTypeStatic() { return EComponentType::WheeledVehicleMovementComponent; };

		void Serialize( Archive& Ar ) override;

		void RegisterComponent( World* InOwningWorld ) override;
		void UnRegisterComponent() override;

		//void SetMovementInput(const Vector& Input);

		//inline Vector GetPosition() const { return m_Controller ? Pd2Vector(m_Controller->getFootPosition()) : GetOwningActor()->GetActorLocation(); }

		//void SendPhysicTranform(const Transform& InTransform);

		//PhysicUserData& GetUserData()
		//{
		//	PhysicUserData::Set<CharacterMovementComponent>((void*)&UserData, const_cast<CharacterMovementComponent*>(this));
		//	return UserData;
		//}

#if WITH_EDITOR
		void DrawDetailPanel( float DeltaTime ) override;
#endif

	protected:

		float Mass;

		//physx::PxController* m_Controller;
		Vector m_MovementInput;

		//PhysicUserData UserData;
	};
}