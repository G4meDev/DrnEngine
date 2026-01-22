#pragma once

#include "ForwardTypes.h"
#include "Runtime/Engine/Component.h"

#include "vehicle2/PxVehicleAPI.h"

#define NUM_WHEELS 4

namespace Drn
{
	using namespace physx::vehicle2;
	class WheeledVehiclePawn;

	//struct BaseVehicleParams
	//{
	//	PxVehicleAxleDescription axleDescription;
	//	PxVehicleFrame frame;
	//	PxVehicleScale scale;
	//	PxVehicleSuspensionStateCalculationParams suspensionStateCalculationParams;
	//
	//	//Command response
	//	PxVehicleBrakeCommandResponseParams brakeResponseParams[2];
	//	PxVehicleSteerCommandResponseParams steerResponseParams;
	//	PxVehicleAckermannParams ackermannParams[1];
	//
	//	//Suspension
	//	PxVehicleSuspensionParams suspensionParams[PxVehicleLimits::eMAX_NB_WHEELS];
	//	PxVehicleSuspensionComplianceParams suspensionComplianceParams[PxVehicleLimits::eMAX_NB_WHEELS];
	//	PxVehicleSuspensionForceParams suspensionForceParams[PxVehicleLimits::eMAX_NB_WHEELS];
	//
	//	//Tires
	//	PxVehicleTireForceParams tireForceParams[PxVehicleLimits::eMAX_NB_WHEELS];
	//
	//	//Wheels
	//	PxVehicleWheelParams wheelParams[PxVehicleLimits::eMAX_NB_WHEELS];
	//
	//	//Rigid body
	//	PxVehicleRigidBodyParams rigidBodyParams;
	//
	//	BaseVehicleParams transformAndScale(
	//	const PxVehicleFrame& srcFrame, const PxVehicleFrame& trgFrame, const PxVehicleScale& srcScale, const PxVehicleScale& trgScale) const;
	//
	//	PX_FORCE_INLINE bool isValid() const
	//	{
	//		if (!axleDescription.isValid())
	//			return false;
	//		if (!frame.isValid())
	//			return true;
	//		if (!scale.isValid())
	//			return false;
	//		if (!suspensionStateCalculationParams.isValid())
	//			return false;
	//
	//		if (!brakeResponseParams[0].isValid(axleDescription))
	//			return false;
	//		if (!brakeResponseParams[1].isValid(axleDescription))
	//			return false;
	//		if (!steerResponseParams.isValid(axleDescription))
	//			return false;
	//		if (!ackermannParams[0].isValid(axleDescription))
	//				return false;
	//
	//		for (PxU32 i = 0; i < axleDescription.nbWheels; i++)
	//		{
	//			const PxU32 wheelId = axleDescription.wheelIdsInAxleOrder[i];
	//
	//			if (!suspensionParams[wheelId].isValid())
	//				return false;
	//			if (!suspensionComplianceParams[wheelId].isValid())
	//				return false;
	//			if (!suspensionForceParams[wheelId].isValid())
	//				return false;
	//
	//			if (!tireForceParams[wheelId].isValid())
	//				return false;
	//
	//			if (!wheelParams[wheelId].isValid())
	//				return false;
	//		}
	//
	//		if (!rigidBodyParams.isValid())
	//			return false;
	//
	//		return true;
	//	}
	//};
	//
	//struct BaseVehicleState
	//{
	//	//Command responses
	//	PxReal brakeCommandResponseStates[PxVehicleLimits::eMAX_NB_WHEELS];
	//	PxReal steerCommandResponseStates[PxVehicleLimits::eMAX_NB_WHEELS];
	//	PxVehicleWheelActuationState actuationStates[PxVehicleLimits::eMAX_NB_WHEELS];
	//
	//	//Road geometry
	//	PxVehicleRoadGeometryState roadGeomStates[PxVehicleLimits::eMAX_NB_WHEELS];
	//
	//	//Suspensions
	//	PxVehicleSuspensionState suspensionStates[PxVehicleLimits::eMAX_NB_WHEELS];
	//	PxVehicleSuspensionComplianceState suspensionComplianceStates[PxVehicleLimits::eMAX_NB_WHEELS];
	//	PxVehicleSuspensionForce suspensionForces[PxVehicleLimits::eMAX_NB_WHEELS];
	//
	//	//Tires
	//	PxVehicleTireGripState tireGripStates[PxVehicleLimits::eMAX_NB_WHEELS];
	//	PxVehicleTireDirectionState tireDirectionStates[PxVehicleLimits::eMAX_NB_WHEELS];
	//	PxVehicleTireSpeedState tireSpeedStates[PxVehicleLimits::eMAX_NB_WHEELS];
	//	PxVehicleTireSlipState tireSlipStates[PxVehicleLimits::eMAX_NB_WHEELS];
	//	PxVehicleTireCamberAngleState tireCamberAngleStates[PxVehicleLimits::eMAX_NB_WHEELS];
	//	PxVehicleTireStickyState tireStickyStates[PxVehicleLimits::eMAX_NB_WHEELS];
	//	PxVehicleTireForce tireForces[PxVehicleLimits::eMAX_NB_WHEELS];
	//
	//	//Wheels
	//	PxVehicleWheelRigidBody1dState wheelRigidBody1dStates[PxVehicleLimits::eMAX_NB_WHEELS];
	//	PxVehicleWheelLocalPose wheelLocalPoses[PxVehicleLimits::eMAX_NB_WHEELS];
	//
	//	//Rigid body
	//	PxVehicleRigidBodyState rigidBodyState;
	//
	//	PX_FORCE_INLINE void setToDefault()
	//	{
	//		for (unsigned int i = 0; i < PxVehicleLimits::eMAX_NB_WHEELS; i++)
	//		{
	//			brakeCommandResponseStates[i] = 0.0;
	//			steerCommandResponseStates[i] = 0.0f;
	//
	//			actuationStates[i].setToDefault();
	//
	//			roadGeomStates[i].setToDefault();
	//
	//			suspensionStates[i].setToDefault();
	//			suspensionComplianceStates[i].setToDefault();
	//			suspensionForces[i].setToDefault();
	//
	//			tireGripStates[i].setToDefault();
	//			tireDirectionStates[i].setToDefault();
	//			tireSpeedStates[i].setToDefault();
	//			tireSlipStates[i].setToDefault();
	//			tireCamberAngleStates[i].setToDefault();
	//			tireStickyStates[i].setToDefault();
	//			tireForces[i].setToDefault();
	//
	//			wheelRigidBody1dStates[i].setToDefault();
	//			wheelLocalPoses[i].setToDefault();
	//		}
	//
	//		rigidBodyState.setToDefault();
	//	}
	//};

	class WheelData
	{
	public:
		Vector SocketLocation;
		float Radius;
		float HalfWidth;


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

#if WITH_EDITOR
		void DrawDetailPanel( float DeltaTime ) override;
#endif

	protected:

		float ThrottleInput;
		float SteerInput;

		WheeledVehiclePawn* OwningVehicle;

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

// -------------------------------------------------------------------------------------------------

		PxRigidDynamic* RigidBody;
		//PxVehiclePhysXActor physxActor;

		//BaseVehicleParams BaseParams;
		//BaseVehicleState BaseState;
		//
		//PxVehicleCommandState CommandState;
		//PxVehicleEngineDriveTransmissionCommandState TransmissionCommandState;
		//
		//PxVehicleComponentSequence ComponentSequence;
		//uint8 ComponentSequenceSubstepGroupHandle;
	};
}