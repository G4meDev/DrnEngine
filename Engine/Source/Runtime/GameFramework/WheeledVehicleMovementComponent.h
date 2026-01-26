#pragma once

#include "ForwardTypes.h"
#include "Runtime/Engine/Component.h"
#include "Runtime/Engine/CurveFloat.h"

#include "vehicle2/PxVehicleAPI.h"

#define NUM_WHEELS 4

namespace Drn
{
	using namespace physx::vehicle2;
	class WheeledVehiclePawn;
	class StaticMeshComponent;

	struct VehicleParams
	{
		PxVehicleAxleDescription axleDescription;
		PxVehicleFrame frame;
		PxVehicleScale scale;
		PxVehicleSuspensionStateCalculationParams suspensionStateCalculationParams;
	
		//Command response
		PxVehicleBrakeCommandResponseParams brakeResponseParams[2];
		PxVehicleSteerCommandResponseParams steerResponseParams;
		PxVehicleAckermannParams ackermannParams[1];
	
		//Suspension
		PxVehicleSuspensionParams suspensionParams[PxVehicleLimits::eMAX_NB_WHEELS];
		PxVehicleSuspensionComplianceParams suspensionComplianceParams[PxVehicleLimits::eMAX_NB_WHEELS];
		PxVehicleSuspensionForceParams suspensionForceParams[PxVehicleLimits::eMAX_NB_WHEELS];
	
		//Tires
		PxVehicleTireForceParams tireForceParams[PxVehicleLimits::eMAX_NB_WHEELS];
	
		//Wheels
		PxVehicleWheelParams wheelParams[PxVehicleLimits::eMAX_NB_WHEELS];
	
		//Rigid body
		PxVehicleRigidBodyParams rigidBodyParams;
	
		PxVehiclePhysXRoadGeometryQueryParams physxRoadGeometryQueryParams;
		PxVehiclePhysXMaterialFrictionParams physxMaterialFrictionParams[PxVehicleLimits::eMAX_NB_WHEELS];
		PxVehiclePhysXSuspensionLimitConstraintParams physxSuspensionLimitConstraintParams[PxVehicleLimits::eMAX_NB_WHEELS];
		PxTransform physxActorCMassLocalPose;
		PxVec3 physxActorBoxShapeHalfExtents;
		PxTransform physxActorBoxShapeLocalPose;
		PxTransform physxWheelShapeLocalPoses[PxVehicleLimits::eMAX_NB_WHEELS];

		PxVehicleAutoboxParams autoboxParams;
		PxVehicleClutchCommandResponseParams clutchCommandResponseParams;
		PxVehicleEngineParams engineParams;
		PxVehicleGearboxParams gearBoxParams;
		//PxVehicleMultiWheelDriveDifferentialParams multiWheelDifferentialParams;
		PxVehicleFourWheelDriveDifferentialParams fourWheelDifferentialParams;
		//PxVehicleTankDriveDifferentialParams tankDifferentialParams;
		PxVehicleClutchParams clutchParams;

		VehicleParams transformAndScale(
		const PxVehicleFrame& srcFrame, const PxVehicleFrame& trgFrame, const PxVehicleScale& srcScale, const PxVehicleScale& trgScale) const;
	
		PX_FORCE_INLINE bool isValid() const
		{
			if (!axleDescription.isValid())
				return false;
			if (!frame.isValid())
				return true;
			if (!scale.isValid())
				return false;
			if (!suspensionStateCalculationParams.isValid())
				return false;
	
			if (!brakeResponseParams[0].isValid(axleDescription))
				return false;
			if (!brakeResponseParams[1].isValid(axleDescription))
				return false;
			if (!steerResponseParams.isValid(axleDescription))
				return false;
			if (!ackermannParams[0].isValid(axleDescription))
					return false;
	
			for (PxU32 i = 0; i < axleDescription.nbWheels; i++)
			{
				const PxU32 wheelId = axleDescription.wheelIdsInAxleOrder[i];
	
				if (!suspensionParams[wheelId].isValid())
					return false;
				if (!suspensionComplianceParams[wheelId].isValid())
					return false;
				if (!suspensionForceParams[wheelId].isValid())
					return false;
	
				if (!tireForceParams[wheelId].isValid())
					return false;
	
				if (!wheelParams[wheelId].isValid())
					return false;
			}
	
			if (!rigidBodyParams.isValid())
				return false;
	
			return true;
		}
	};
	
	struct VehicleState
	{
		//Command responses
		PxReal brakeCommandResponseStates[PxVehicleLimits::eMAX_NB_WHEELS];
		PxReal steerCommandResponseStates[PxVehicleLimits::eMAX_NB_WHEELS];
		PxVehicleWheelActuationState actuationStates[PxVehicleLimits::eMAX_NB_WHEELS];
	
		//Road geometry
		PxVehicleRoadGeometryState roadGeomStates[PxVehicleLimits::eMAX_NB_WHEELS];
	
		//Suspensions
		PxVehicleSuspensionState suspensionStates[PxVehicleLimits::eMAX_NB_WHEELS];
		PxVehicleSuspensionComplianceState suspensionComplianceStates[PxVehicleLimits::eMAX_NB_WHEELS];
		PxVehicleSuspensionForce suspensionForces[PxVehicleLimits::eMAX_NB_WHEELS];
	
		//Tires
		PxVehicleTireGripState tireGripStates[PxVehicleLimits::eMAX_NB_WHEELS];
		PxVehicleTireDirectionState tireDirectionStates[PxVehicleLimits::eMAX_NB_WHEELS];
		PxVehicleTireSpeedState tireSpeedStates[PxVehicleLimits::eMAX_NB_WHEELS];
		PxVehicleTireSlipState tireSlipStates[PxVehicleLimits::eMAX_NB_WHEELS];
		PxVehicleTireCamberAngleState tireCamberAngleStates[PxVehicleLimits::eMAX_NB_WHEELS];
		PxVehicleTireStickyState tireStickyStates[PxVehicleLimits::eMAX_NB_WHEELS];
		PxVehicleTireForce tireForces[PxVehicleLimits::eMAX_NB_WHEELS];
	
		//Wheels
		PxVehicleWheelRigidBody1dState wheelRigidBody1dStates[PxVehicleLimits::eMAX_NB_WHEELS];
		PxVehicleWheelLocalPose wheelLocalPoses[PxVehicleLimits::eMAX_NB_WHEELS];
	
		//Rigid body
		PxVehicleRigidBodyState rigidBodyState;
	
		PxVehiclePhysXActor physxActor;					//physx actor
		PxVehiclePhysXSteerState physxSteerState;
		PxVehiclePhysXConstraints physxConstraints;		//susp limit and sticky tire constraints

		PxVehicleEngineDriveThrottleCommandResponseState throttleCommandResponseState;
		PxVehicleAutoboxState autoboxState;
		PxVehicleClutchCommandResponseState clutchCommandResponseState;
		PxVehicleDifferentialState differentialState;
		PxVehicleWheelConstraintGroupState wheelConstraintGroupState;
		PxVehicleEngineState engineState;
		PxVehicleGearboxState gearboxState;
		PxVehicleClutchSlipState clutchState;

		PX_FORCE_INLINE void setToDefault()
		{
			for (unsigned int i = 0; i < PxVehicleLimits::eMAX_NB_WHEELS; i++)
			{
				brakeCommandResponseStates[i] = 0.0;
				steerCommandResponseStates[i] = 0.0f;
	
				actuationStates[i].setToDefault();
	
				roadGeomStates[i].setToDefault();
	
				suspensionStates[i].setToDefault();
				suspensionComplianceStates[i].setToDefault();
				suspensionForces[i].setToDefault();
	
				tireGripStates[i].setToDefault();
				tireDirectionStates[i].setToDefault();
				tireSpeedStates[i].setToDefault();
				tireSlipStates[i].setToDefault();
				tireCamberAngleStates[i].setToDefault();
				tireStickyStates[i].setToDefault();
				tireForces[i].setToDefault();
				
				wheelRigidBody1dStates[i].setToDefault();
				wheelLocalPoses[i].setToDefault();
			}
	
			rigidBodyState.setToDefault();

			physxActor.setToDefault();
			physxSteerState.setToDefault();
			physxConstraints.setToDefault();

			throttleCommandResponseState.setToDefault();
			autoboxState.setToDefault();
			clutchCommandResponseState.setToDefault();
			differentialState.setToDefault();
			wheelConstraintGroupState.setToDefault();
			engineState.setToDefault();
			gearboxState.setToDefault();
			clutchState.setToDefault();
		}
	};

	class VehicleCommandsBase
		: public PxVehicleRigidBodyComponent
		, public PxVehicleEngineDriveCommandResponseComponent
		, public PxVehiclePhysXActorBeginComponent
		, public PxVehiclePhysXActorEndComponent
		, public PxVehiclePhysXRoadGeometrySceneQueryComponent
		, public PxVehicleFourWheelDriveDifferentialStateComponent
		, public PxVehicleEngineDrivetrainComponent
		, public PxVehicleEngineDriveActuationStateComponent
		, public PxVehicleSuspensionComponent
		, public PxVehicleTireComponent
		, public PxVehicleWheelComponent
		, public PxVehiclePhysXConstraintComponent
	{
	public:
		
		VehicleCommandsBase(class WheeledVehicleMovementComponent* inOwningVehicle) : OwningVehicle(inOwningVehicle) {}
		class WheeledVehicleMovementComponent* OwningVehicle;

		void getDataForRigidBodyComponent( const PxVehicleAxleDescription*& axleDescription, const PxVehicleRigidBodyParams*& rigidBodyParams,
			PxVehicleArrayData<const PxVehicleSuspensionForce>& suspensionForces, PxVehicleArrayData<const PxVehicleTireForce>& tireForces,
			const PxVehicleAntiRollTorque*& antiRollTorque, PxVehicleRigidBodyState*& rigidBodyState ) override;

		void getDataForEngineDriveCommandResponseComponent( const PxVehicleAxleDescription*& axleDescription, PxVehicleSizedArrayData<const PxVehicleBrakeCommandResponseParams>& brakeResponseParams,
			const PxVehicleSteerCommandResponseParams*& steerResponseParams, PxVehicleSizedArrayData<const PxVehicleAckermannParams>& ackermannParams,
			const PxVehicleGearboxParams*& gearboxParams, const PxVehicleClutchCommandResponseParams*& clutchResponseParams, const PxVehicleEngineParams*& engineParams,
			const PxVehicleRigidBodyState*& rigidBodyState, const PxVehicleEngineState*& engineState, const PxVehicleAutoboxParams*& autoboxParams,
			const PxVehicleCommandState*& commands, const PxVehicleEngineDriveTransmissionCommandState*& transmissionCommands,
			PxVehicleArrayData<PxReal>& brakeResponseStates, PxVehicleEngineDriveThrottleCommandResponseState*& throttleResponseState,
			PxVehicleArrayData<PxReal>& steerResponseStates, PxVehicleGearboxState*& gearboxResponseState,
			PxVehicleClutchCommandResponseState*& clutchResponseState, PxVehicleAutoboxState*& autoboxState ) override;

		void getDataForPhysXActorEndComponent( const PxVehicleAxleDescription*& axleDescription, const PxVehicleRigidBodyState*& rigidBodyState,
			PxVehicleArrayData<const PxVehicleWheelParams>& wheelParams, PxVehicleArrayData<const PxTransform>& wheelShapeLocalPoses,
			PxVehicleArrayData<const PxVehicleWheelRigidBody1dState>& wheelRigidBody1dStates, PxVehicleArrayData<const PxVehicleWheelLocalPose>& wheelLocalPoses,
			const PxVehicleGearboxState*& gearState, const PxReal*& throttle, PxVehiclePhysXActor*& physxActor ) override;

		void getDataForPhysXActorBeginComponent( const PxVehicleAxleDescription*& axleDescription, const PxVehicleCommandState*& commands,
			const PxVehicleEngineDriveTransmissionCommandState*& transmissionCommands,
			const PxVehicleGearboxParams*& gearParams, const PxVehicleGearboxState*& gearState,
			const PxVehicleEngineParams*& engineParams, PxVehiclePhysXActor*& physxActor,
			PxVehiclePhysXSteerState*& physxSteerState, PxVehiclePhysXConstraints*& physxConstraints,
			PxVehicleRigidBodyState*& rigidBodyState,
			PxVehicleArrayData<PxVehicleWheelRigidBody1dState>& wheelRigidBody1dStates,
			PxVehicleEngineState*& engineState ) override;

		void getDataForSuspensionComponent( const PxVehicleAxleDescription*& axleDescription, const PxVehicleRigidBodyParams*& rigidBodyParams,
			const PxVehicleSuspensionStateCalculationParams*& suspensionStateCalculationParams, PxVehicleArrayData<const PxReal>& steerResponseStates,
			const PxVehicleRigidBodyState*& rigidBodyState, PxVehicleArrayData<const PxVehicleWheelParams>& wheelParams,
			PxVehicleArrayData<const PxVehicleSuspensionParams>& suspensionParams, PxVehicleArrayData<const PxVehicleSuspensionComplianceParams>& suspensionComplianceParams,
			PxVehicleArrayData<const PxVehicleSuspensionForceParams>& suspensionForceParams, PxVehicleSizedArrayData<const PxVehicleAntiRollForceParams>& antiRollForceParams,
			PxVehicleArrayData<const PxVehicleRoadGeometryState>& wheelRoadGeomStates, PxVehicleArrayData<PxVehicleSuspensionState>& suspensionStates,
			PxVehicleArrayData<PxVehicleSuspensionComplianceState>& suspensionComplianceStates, PxVehicleArrayData<PxVehicleSuspensionForce>& suspensionForces,
			PxVehicleAntiRollTorque*& antiRollTorque ) override;

		void getDataForWheelComponent( const PxVehicleAxleDescription*& axleDescription,
			PxVehicleArrayData<const PxReal>& steerResponseStates, PxVehicleArrayData<const PxVehicleWheelParams>& wheelParams,
			PxVehicleArrayData<const PxVehicleSuspensionParams>& suspensionParams, PxVehicleArrayData<const PxVehicleWheelActuationState>& actuationStates,
			PxVehicleArrayData<const PxVehicleSuspensionState>& suspensionStates, PxVehicleArrayData<const PxVehicleSuspensionComplianceState>& suspensionComplianceStates,
			PxVehicleArrayData<const PxVehicleTireSpeedState>& tireSpeedStates, PxVehicleArrayData<PxVehicleWheelRigidBody1dState>& wheelRigidBody1dStates,
			PxVehicleArrayData<PxVehicleWheelLocalPose>& wheelLocalPoses ) override;

		void getDataForPhysXConstraintComponent( const PxVehicleAxleDescription*& axleDescription, const PxVehicleRigidBodyState*& rigidBodyState,
			PxVehicleArrayData<const PxVehicleSuspensionParams>& suspensionParams, PxVehicleArrayData<const PxVehiclePhysXSuspensionLimitConstraintParams>& suspensionLimitParams,
			PxVehicleArrayData<const PxVehicleSuspensionState>& suspensionStates, PxVehicleArrayData<const PxVehicleSuspensionComplianceState>& suspensionComplianceStates,
			PxVehicleArrayData<const PxVehicleRoadGeometryState>& wheelRoadGeomStates, PxVehicleArrayData<const PxVehicleTireDirectionState>& tireDirectionStates,
			PxVehicleArrayData<const PxVehicleTireStickyState>& tireStickyStates, PxVehiclePhysXConstraints*& constraints ) override;

		void getDataForPhysXRoadGeometrySceneQueryComponent( const PxVehicleAxleDescription*& axleDescription, const PxVehiclePhysXRoadGeometryQueryParams*& roadGeomParams,
			PxVehicleArrayData<const PxReal>& steerResponseStates, const PxVehicleRigidBodyState*& rigidBodyState, PxVehicleArrayData<const PxVehicleWheelParams>& wheelParams,
			PxVehicleArrayData<const PxVehicleSuspensionParams>& suspensionParams, PxVehicleArrayData<const PxVehiclePhysXMaterialFrictionParams>& materialFrictionParams,
			PxVehicleArrayData<PxVehicleRoadGeometryState>& roadGeometryStates, PxVehicleArrayData<PxVehiclePhysXRoadGeometryQueryState>& physxRoadGeometryStates ) override;

		void getDataForFourWheelDriveDifferentialStateComponent( const PxVehicleAxleDescription*& axleDescription, const PxVehicleFourWheelDriveDifferentialParams*& differentialParams,
			PxVehicleArrayData<const PxVehicleWheelRigidBody1dState>& wheelRigidbody1dStates, PxVehicleDifferentialState*& differentialState,
			PxVehicleWheelConstraintGroupState*& wheelConstraintGroupState ) override;

		void getDataForEngineDrivetrainComponent( const PxVehicleAxleDescription*& axleDescription, PxVehicleArrayData<const PxVehicleWheelParams>& wheelParams,
			const PxVehicleEngineParams*& engineParams, const PxVehicleClutchParams*& clutchParams, const PxVehicleGearboxParams*& gearboxParams, PxVehicleArrayData<const PxReal>& brakeResponseStates,
			PxVehicleArrayData<const PxVehicleWheelActuationState>&  actuationStates, PxVehicleArrayData<const PxVehicleTireForce>& tireForces,
			const PxVehicleEngineDriveThrottleCommandResponseState*& throttleResponseState, const PxVehicleClutchCommandResponseState*& clutchResponseState,
			const PxVehicleDifferentialState*& differentialState, const PxVehicleWheelConstraintGroupState*& constraintGroupState,
			PxVehicleArrayData<PxVehicleWheelRigidBody1dState>& wheelRigidBody1dStates, PxVehicleEngineState*& engineState, PxVehicleGearboxState*& gearboxState,
			PxVehicleClutchSlipState*& clutchState ) override;

		void getDataForTireComponent( const PxVehicleAxleDescription*& axleDescription, PxVehicleArrayData<const PxReal>& steerResponseStates,
			const PxVehicleRigidBodyState*& rigidBodyState, PxVehicleArrayData<const PxVehicleWheelActuationState>& actuationStates,
			PxVehicleArrayData<const PxVehicleWheelParams>& wheelParams, PxVehicleArrayData<const PxVehicleSuspensionParams>& suspensionParams,
			PxVehicleArrayData<const PxVehicleTireForceParams>& tireForceParams, PxVehicleArrayData<const PxVehicleRoadGeometryState>& roadGeomStates,
			PxVehicleArrayData<const PxVehicleSuspensionState>& suspensionStates, PxVehicleArrayData<const PxVehicleSuspensionComplianceState>& suspensionComplianceStates,
			PxVehicleArrayData<const PxVehicleSuspensionForce>& suspensionForces, PxVehicleArrayData<const PxVehicleWheelRigidBody1dState>& wheelRigidBody1DStates,
			PxVehicleArrayData<PxVehicleTireGripState>& tireGripStates, PxVehicleArrayData<PxVehicleTireDirectionState>& tireDirectionStates,
			PxVehicleArrayData<PxVehicleTireSpeedState>& tireSpeedStates, PxVehicleArrayData<PxVehicleTireSlipState>& tireSlipStates,
			PxVehicleArrayData<PxVehicleTireCamberAngleState>& tireCamberAngleStates, PxVehicleArrayData<PxVehicleTireStickyState>& tireStickyStates,
			PxVehicleArrayData<PxVehicleTireForce>& tireForces ) override;

		void getDataForEngineDriveActuationStateComponent( const PxVehicleAxleDescription*& axleDescription, const PxVehicleGearboxParams*& gearboxParams,
			PxVehicleArrayData<const PxReal>& brakeResponseStates, const PxVehicleEngineDriveThrottleCommandResponseState*& throttleResponseState,
			const PxVehicleGearboxState*& gearboxState, const PxVehicleDifferentialState*& differentialState,
			const PxVehicleClutchCommandResponseState*& clutchResponseState, PxVehicleArrayData<PxVehicleWheelActuationState>& actuationStates ) override;
        };

	class WheelData
	{
	public:
		Vector SocketLocation;
		float Radius;
		float HalfWidth;
		float Mass;
		float DampingRate;

		float SusppensionLength;
		float SusppensionDamping;
		float SusppensionStrength;
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

		//inline void SetOwningVehicle( WheeledVehiclePawn* InOwningVehicle ) { OwningVehicle = InOwningVehicle; }
		inline void SetVehicleBody( StaticMeshComponent* InVehicleBody ) { VehicleBody = InVehicleBody; }
		inline void SetThrottleInput(float InThrottleInput) { ThrottleInput = InThrottleInput; }
		inline void SetSteerInput(float InSteerInput) { SteerInput = InSteerInput; }

		Transform GetWheelWorldTransform(int32 WheelIndex) const;

#if WITH_EDITOR
		void DrawDetailPanel( float DeltaTime ) override;
#endif

	protected:

		inline float InterpInputValue( float DeltaTime, float CurrentValue, float NewValue, float RiseRate, float FallRate ) const
		{
			const float DeltaValue = NewValue - CurrentValue;
			const bool bRising = (( DeltaValue > 0.0f ) == ( CurrentValue > 0.0f )) ||
									(( DeltaValue != 0.f ) && ( CurrentValue == 0.f ));

			const float MaxDeltaValue = DeltaTime * ( bRising ? RiseRate : FallRate );
			const float ClampedDeltaValue = std::clamp( DeltaValue, -MaxDeltaValue, MaxDeltaValue );
			return CurrentValue + ClampedDeltaValue;
		}

		//WheeledVehiclePawn* OwningVehicle;
		StaticMeshComponent* VehicleBody;
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

		float ThrottleInput;
		float SteerInput;

		float LastThrottleInput;
		float LastSteerInput;

		float ThrottleInputRiseRate = 6.0f;
		float ThrottleInputFallRate = 10.0f;

		float SteerInputRiseRate = 2.0f;
		float SteerInputFallRate = 5.0f;

		CurveFloat SteerCurve;

// -------------------------------------------------------------------------------------------------

		const PxVec3 Gravity = PxVec3( 0.0f, -9.81f, 0.0f );
		PxVehiclePhysXActor PhysxActor;
		
		PxVehiclePhysXSimulationContext SimulationContext;
		PxVehicleComponentSequence ComponentSequence;
		uint8 ComponentSequenceSubstepGroupHandle;

		VehicleCommandsBase VehicleCommands;
		PxVehicleCommandState CommandState;
		PxVehicleEngineDriveTransmissionCommandState TransmissionCommandState;
		VehicleParams VehicleParams;
		VehicleState VehicleState;

		// TODO: do table lookup
		PxVehiclePhysXMaterialFriction gPhysXMaterialFrictions[16];
		int32 gNbPhysXMaterialFrictions = 1;

		friend class VehicleCommandsBase;
	};
}