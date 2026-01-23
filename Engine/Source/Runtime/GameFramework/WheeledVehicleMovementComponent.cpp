#include "DrnPCH.h"
#include "WheeledVehicleMovementComponent.h"

namespace Drn
{
	WheeledVehicleMovementComponent::WheeledVehicleMovementComponent()
		: Component()
		, OwningVehicle(nullptr)
		, VehicleCommands(this)
		, SteerInput(0.0f)
		, ThrottleInput(0.0f)
	{
		PhysxActor.setToDefault();
		CommandState.setToDefault();
		VehicleState.setToDefault();

		// TODO: remove
		gPhysXMaterialFrictions[0].friction = 1.0f;
		gPhysXMaterialFrictions[0].material = PhysicManager::Get()->TempMaterial;

		//Vector SocketOffset = Vector(2.25, 0.98f, 2.5);
		Vector SocketOffset = Vector(2.25, 0.0f, 2.5);

		FrontLeftWheel.SocketLocation = SocketOffset * Vector(-1, 1, 1);
		FrontRightWheel.SocketLocation = SocketOffset * Vector(1, 1, 1);
		RearLeftWheel.SocketLocation = SocketOffset * Vector(-1, 1, -1);
		RearRightWheel.SocketLocation = SocketOffset * Vector(1, 1, -1);

		for (int32 i = 0; i < NUM_WHEELS; i++)
		{
			Wheels[i].Radius = 0.8f;
			Wheels[i].HalfWidth = 0.2f;
			Wheels[i].Mass = 10.0f;
			Wheels[i].DampingRate = 1000.0f;
		}
	}

	WheeledVehicleMovementComponent::~WheeledVehicleMovementComponent()
	{
		
	}

	void WheeledVehicleMovementComponent::Tick( float DeltaTime )
	{
		Component::Tick(DeltaTime);

		drn_check(OwningVehicle);

		if (PhysxActor.rigidBody)
		{
			CommandState.throttle = ThrottleInput;
			CommandState.steer = SteerInput;

			ComponentSequence.update(DeltaTime, SimulationContext);
		}

		const bool bValidBody = OwningVehicle->GetVehicleBody()->GetMesh().IsValid();
		const Vector SpringDirection = GetOwningActor()->GetActorUpVector();
		const float BodyMass = bValidBody ? OwningVehicle->GetVehicleBody()->GetBodyInstance().GetMass() : 1.0f;

		if (bValidBody)
		{

		}

		ThrottleInput = 0;
		SteerInput = 0;
	}

	void WheeledVehicleMovementComponent::Serialize( Archive& Ar )
	{
		Component::Serialize(Ar);

		if (Ar.IsLoading())
		{
		}
		else
		{
		}
	}

	void WheeledVehicleMovementComponent::RegisterComponent( World* InOwningWorld )
	{
		Component::RegisterComponent(InOwningWorld);

		//drn_check(BaseParams.isValid());

		PxRigidActor* RigidActor = OwningVehicle->GetVehicleBody()->GetBodyInstance().GetRigidActor();
		PhysxActor.rigidBody = RigidActor ? RigidActor->is<PxRigidDynamic>() : nullptr;

		if (PhysxActor.rigidBody)
		{
			PxRigidBody* RigidBody = PhysxActor.rigidBody;

			RigidBody->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, false);
			RigidBody->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
			RigidBody->setName("VehicleRigidBody");
			//RigidBody->setCMassLocalPose(rigidActorCmassLocalPose);
			//RigidBody->setMass(rigidActorParams.rigidBodyParams.mass);
			//RigidBody->setMassSpaceInertiaTensor(rigidActorParams.rigidBodyParams.moi);

			for (PxU32 i = 0; i < 4; i++)
			{
				const PxF32 radius = Wheels[i].Radius;
				const PxF32 halfWidth = Wheels[i].HalfWidth;

				PxVec3 verts[32];
				for (PxU32 k = 0; k < 16; k++)
				{
					const PxF32 lng = radius * PxCos(k*2.0f*PxPi / 16.0f);
					const PxF32 lat = halfWidth;
					const PxF32 vrt = radius * PxSin(k*2.0f*PxPi / 16.0f);

					const PxVehicleFrame VehicleFrame = PhysicManager::Get()->GetDefaultVehicleFrame();
					const PxVec3 pos0 = VehicleFrame.getFrame()*PxVec3(lng, lat, vrt);
					const PxVec3 pos1 = VehicleFrame.getFrame()*PxVec3(lng, -lat, vrt);
					verts[2 * k + 0] = pos0;
					verts[2 * k + 1] = pos1;
				}

				// Create descriptor for convex mesh
				PxConvexMeshDesc convexDesc;
				convexDesc.points.count = 32;
				convexDesc.points.stride = sizeof(PxVec3);
				convexDesc.points.data = verts;
				convexDesc.flags = PxConvexFlag::eCOMPUTE_CONVEX;

				PxConvexMesh* convexMesh = NULL;
				PxDefaultMemoryOutputStream buf;

				PxCookingParams CookParams = PxCookingParams(physx::PxTolerancesScale());

				if (PxCookConvexMesh(CookParams, convexDesc, buf))
				{
					PxDefaultMemoryInputData id(buf.getData(), buf.getSize());
					convexMesh = PhysicManager::Get()->GetPhysics()->createConvexMesh(id);
				}

				PxConvexMeshGeometry convexMeshGeom(convexMesh);
				PxShape* wheelShape = PhysicManager::Get()->GetPhysics()->createShape(convexMeshGeom, *PhysicManager::Get()->TempMaterial, true);
				//wheelShape->setFlags(wheelShapeParams.flags);
				//wheelShape->setSimulationFilterData(wheelShapeParams.simulationFilterData);
				//wheelShape->setQueryFilterData(wheelShapeParams.queryFilterData);

				RigidBody->attachShape(*wheelShape);
				wheelShape->release();
				convexMesh->release();

				PhysxActor.wheelShapes[i] = wheelShape;
			}

			{
				SimulationContext.setToDefault();
				SimulationContext.frame = PhysicManager::Get()->GetDefaultVehicleFrame();
				SimulationContext.scale.scale = 1.0f;
				SimulationContext.gravity = Gravity;
				SimulationContext.physxScene = GetWorld()->GetPhysicScene()->GetPhysxScene();
				SimulationContext.physxActorUpdateMode = PxVehiclePhysXActorUpdateMode::eAPPLY_ACCELERATION;
			}

			{
				VehicleParams.axleDescription.setToDefault();

				uint32 FrontWheelsIndex[2] = {0, 1};
				VehicleParams.axleDescription.addAxle(2, FrontWheelsIndex);

				uint32 RearWheelsIndex[2] = {2, 3};
				VehicleParams.axleDescription.addAxle(2, RearWheelsIndex);

				VehicleParams.rigidBodyParams.mass = RigidBody->getMass();

				// TODO: ???
				//VehicleParams.rigidBodyParams.moi = RigidBody->getMassSpaceInertiaTensor();
				VehicleParams.rigidBodyParams.moi = PxVec3(1);

				for (int32 i = 0; i < 4; i++)
				{
					VehicleParams.wheelParams[i].radius = Wheels[i].Radius;
					VehicleParams.wheelParams[i].halfWidth = Wheels[i].HalfWidth;
					VehicleParams.wheelParams[i].mass = Wheels[i].Mass;
					VehicleParams.wheelParams[i].dampingRate = Wheels[i].DampingRate;

					// TODO: ???
					VehicleParams.wheelParams[i].moi = 1;
				}

				PxVehicleConstraintsCreate(VehicleParams.axleDescription, *PhysicManager::Get()->GetPhysics(), *PhysxActor.rigidBody, VehicleState.physxConstraints);
			}

			{
				VehicleParams.physxRoadGeometryQueryParams.roadGeometryQueryType = PxVehiclePhysXRoadGeometryQueryType::eRAYCAST;
				//VehicleParams.physxRoadGeometryQueryParams.defaultFilterData = queryFilterData;
				//VehicleParams.physxRoadGeometryQueryParams.filterCallback = queryFilterCallback;
				VehicleParams.physxRoadGeometryQueryParams.filterDataEntries = NULL;

				const float DefaultFriction = 1.0f;

				for(PxU32 i = 0; i < VehicleParams.axleDescription.nbWheels; i++)
				{
					const PxU32  wheelId = VehicleParams.axleDescription.wheelIdsInAxleOrder[i];
					VehicleParams.physxMaterialFrictionParams[wheelId].defaultFriction = DefaultFriction;
					VehicleParams.physxMaterialFrictionParams[wheelId].materialFrictions = gPhysXMaterialFrictions;
					VehicleParams.physxMaterialFrictionParams[wheelId].nbMaterialFrictions = gNbPhysXMaterialFrictions;

					VehicleParams.physxSuspensionLimitConstraintParams[wheelId].restitution = 0.0f;
					VehicleParams.physxSuspensionLimitConstraintParams[wheelId].directionForSuspensionLimitConstraint = PxVehiclePhysXSuspensionLimitConstraintParams::eROAD_GEOMETRY_NORMAL;

					//VehicleParams.physxWheelShapeLocalPoses[wheelId] = PxTransform(PxIdentity);
					VehicleParams.physxWheelShapeLocalPoses[wheelId] = PxTransform(Vector2P(Wheels[i].SocketLocation));
				}
				
				VehicleParams.physxActorCMassLocalPose = RigidBody->getCMassLocalPose();
				//VehicleParams.physxActorBoxShapeHalfExtents = actorBoxShapeHalfExtents;
				//VehicleParams.physxActorBoxShapeLocalPose = actorBoxShapeLocalPose;
			}

			{
				ComponentSequence.add(static_cast<PxVehiclePhysXActorBeginComponent*>(&VehicleCommands));

				//ComponentSequence.add(static_cast<PxVehicleEngineDriveCommandResponseComponent*>(&VehicleCommands));
				//ComponentSequence.add(static_cast<PxVehicleFourWheelDriveDifferentialStateComponent*>(this));
				//ComponentSequence.add(static_cast<PxVehicleEngineDriveActuationStateComponent*>(this));
				//ComponentSequence.add(static_cast<PxVehiclePhysXRoadGeometrySceneQueryComponent*>(this));
				
				{
					ComponentSequenceSubstepGroupHandle = ComponentSequence.beginSubstepGroup(3);

					//ComponentSequence.add(static_cast<PxVehicleSuspensionComponent*>(this));
					//ComponentSequence.add(static_cast<PxVehicleTireComponent*>(this));
					//ComponentSequence.add(static_cast<PxVehiclePhysXConstraintComponent*>(this));
					//ComponentSequence.add(static_cast<PxVehicleEngineDrivetrainComponent*>(this));
					ComponentSequence.add(static_cast<PxVehicleRigidBodyComponent*>(&VehicleCommands));

					ComponentSequence.endSubstepGroup();
				}

				//ComponentSequence.add(static_cast<PxVehicleWheelComponent*>(this));

				ComponentSequence.add(static_cast<PxVehiclePhysXActorEndComponent*>(&VehicleCommands));
			}
		}
	}

	void WheeledVehicleMovementComponent::UnRegisterComponent()
	{
		// Rigid body owner is vehicle body in vehicle actor. no need to release rigid body

		Component::UnRegisterComponent();
	}


#if WITH_EDITOR
	void WheeledVehicleMovementComponent::DrawDetailPanel( float DeltaTime )
	{
		Component::DrawDetailPanel(DeltaTime);

	}

#endif

// ---------------------------------------------------------------------------------------------------------------------------------------

	void VehicleCommandsBase::getDataForRigidBodyComponent(const PxVehicleAxleDescription*& axleDescription, const PxVehicleRigidBodyParams*& rigidBodyParams,
		PxVehicleArrayData<const PxVehicleSuspensionForce>& suspensionForces, PxVehicleArrayData<const PxVehicleTireForce>& tireForces,
		const PxVehicleAntiRollTorque*& antiRollTorque, PxVehicleRigidBodyState*& rigidBodyState)
	{
		VehicleParams& Params = OwningVehicle->VehicleParams;
		VehicleState& State = OwningVehicle->VehicleState;

		axleDescription = &Params.axleDescription;
		rigidBodyParams = &Params.rigidBodyParams;
		suspensionForces.setData(State.suspensionForces);
		tireForces.setData(State.tireForces);
		antiRollTorque = NULL;
		rigidBodyState = &State.rigidBodyState;
	}

	void VehicleCommandsBase::getDataForEngineDriveCommandResponseComponent( const PxVehicleAxleDescription*& axleDescription, PxVehicleSizedArrayData<const PxVehicleBrakeCommandResponseParams>& brakeResponseParams,
		const PxVehicleSteerCommandResponseParams*& steerResponseParams, PxVehicleSizedArrayData<const PxVehicleAckermannParams>& ackermannParams,
		const PxVehicleGearboxParams*& gearboxParams, const PxVehicleClutchCommandResponseParams*& clutchResponseParams, const PxVehicleEngineParams*& engineParams,
		const PxVehicleRigidBodyState*& rigidBodyState, const PxVehicleEngineState*& engineState, const PxVehicleAutoboxParams*& autoboxParams,
		const PxVehicleCommandState*& commands, const PxVehicleEngineDriveTransmissionCommandState*& transmissionCommands,
		PxVehicleArrayData<PxReal>& brakeResponseStates, PxVehicleEngineDriveThrottleCommandResponseState*& throttleResponseState,
		PxVehicleArrayData<PxReal>& steerResponseStates, PxVehicleGearboxState*& gearboxResponseState,
		PxVehicleClutchCommandResponseState*& clutchResponseState, PxVehicleAutoboxState*& autoboxState )
	{
		
	}

	void VehicleCommandsBase::getDataForPhysXActorEndComponent(const PxVehicleAxleDescription*& axleDescription, const PxVehicleRigidBodyState*& rigidBodyState,
		PxVehicleArrayData<const PxVehicleWheelParams>& wheelParams, PxVehicleArrayData<const PxTransform>& wheelShapeLocalPoses,
		PxVehicleArrayData<const PxVehicleWheelRigidBody1dState>& wheelRigidBody1dStates, PxVehicleArrayData<const PxVehicleWheelLocalPose>& wheelLocalPoses,
		const PxVehicleGearboxState*& gearState, const PxReal*& throttle, PxVehiclePhysXActor*& physxActor)
	{
		VehicleParams& Params = OwningVehicle->VehicleParams;
		VehicleState& State = OwningVehicle->VehicleState;

		axleDescription = &Params.axleDescription;
		rigidBodyState = &State.rigidBodyState;
		wheelParams.setData(Params.wheelParams);
		wheelShapeLocalPoses.setData(Params.physxWheelShapeLocalPoses);
		wheelRigidBody1dStates.setData(State.wheelRigidBody1dStates);
		wheelLocalPoses.setData(State.wheelLocalPoses);
		physxActor = &OwningVehicle->PhysxActor;

		//gearState = &State.gearboxState;
		gearState = NULL;
		throttle = &OwningVehicle->CommandState.throttle;
	}

	void VehicleCommandsBase::getDataForPhysXActorBeginComponent(const PxVehicleAxleDescription*& axleDescription, const PxVehicleCommandState*& commands,
		const PxVehicleEngineDriveTransmissionCommandState*& transmissionCommands,
		const PxVehicleGearboxParams*& gearParams, const PxVehicleGearboxState*& gearState,
		const PxVehicleEngineParams*& engineParams, PxVehiclePhysXActor*& physxActor,
		PxVehiclePhysXSteerState*& physxSteerState, PxVehiclePhysXConstraints*& physxConstraints,
		PxVehicleRigidBodyState*& rigidBodyState,
		PxVehicleArrayData<PxVehicleWheelRigidBody1dState>& wheelRigidBody1dStates,
		PxVehicleEngineState*& engineState)
	{
		VehicleParams& Params = OwningVehicle->VehicleParams;
		VehicleState& State = OwningVehicle->VehicleState;

		axleDescription = &Params.axleDescription;
		commands = &OwningVehicle->CommandState;
		physxActor = &OwningVehicle->PhysxActor;
		physxSteerState = &State.physxSteerState;
		physxConstraints = &State.physxConstraints;
		rigidBodyState = &State.rigidBodyState;
		wheelRigidBody1dStates.setData(State.wheelRigidBody1dStates);

		transmissionCommands = NULL;
		gearParams = NULL;
		gearState = NULL;
		engineParams = NULL;
		engineState = NULL;
	}

        }  // namespace Drn