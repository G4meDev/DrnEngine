#pragma once

#include "ForwardTypes.h"
#include "Runtime/Engine/Component.h"

#define NUM_WHEELS 4

namespace Drn
{
	class WheeledVehiclePawn;

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
			, SteerAngle(35.0f)
			, WheelRadius(0.8f)
			, SuspensionRestLength(1.2f)
			, SpringStrength(1500.0f)
			, SpringDamper(350.0f)
			, ForceHeightOffset(2.0f)
			, bFrontWheel(true)
			, bRightWheel(true)
			, RotationAngle(0.0f)
		{}

		Vector SocketLocation;
		float SteerAngle;
		float WheelRadius;
		float SuspensionRestLength;
		float SpringStrength;
		float SpringDamper;
		float ForceHeightOffset;

		float Offset;
		bool bFrontWheel;
		bool bRightWheel;

		Vector LastLocation;
		bool bOnGround;
		float RotationAngle;

		bool bEffectedByEngine;
		bool bEffectedBySteer;
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

		inline void SetOwningVehicle( WheeledVehiclePawn* InOwningVehicle ) { OwningVehicle = InOwningVehicle; }
		inline void SetThrottleInput(float InThrottleInput) { ThrottleInput = InThrottleInput; }
		inline void SetSteerInput(float InSteerInput) { SteerInput = InSteerInput; }

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

#if WITH_EDITOR
		void DrawDetailPanel( float DeltaTime ) override;
#endif

	protected:

		float Mass;
		float DragConstant = 3.0f;
		float Drag = 0.42f;

		float ThrottleInput;
		float SteerInput;

		WheeledVehiclePawn* OwningVehicle;
	};
}