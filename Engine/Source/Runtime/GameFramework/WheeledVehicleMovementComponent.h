#pragma once

#include "ForwardTypes.h"
#include "Runtime/Engine/Component.h"

#define NUM_WHEELS 4

namespace Drn
{
	enum class EWheelType : uint8
	{
		FrontLeft,
		FrontRight,
		RearLeft,
		RearRight
	};

	struct WheelData
	{
		WheelData()
			: SocketLocation(Vector::ZeroVector)
			, Offset(0)
			, WheelRadius(0.8f)
			, SuspensionRestLength(1.2f)
			, SpringStrength(1000.0f)
			, SpringDamper(250.0f)
			, bFrontWheel(true)
			, bRightWheel(true)
		{}

		Vector SocketLocation;
		float WheelRadius;
		float SuspensionRestLength;
		float SpringStrength;
		float SpringDamper;

		float Offset;
		bool bFrontWheel;
		bool bRightWheel;

		Vector LastLocation;
		bool bOnGround;
		float SuspensionForce;

		//bool bEffectedByEngine;
		//bool bEffectedBySteer;
	};

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

		union
		{
			struct
			{
				WheelData FrontLeftWheel;
				WheelData FrontRightWheel;
				WheelData RearLeftWheel;
				WheelData RearRightWheel;
			};

			WheelData Wheels[NUM_WHEELS];
		};

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