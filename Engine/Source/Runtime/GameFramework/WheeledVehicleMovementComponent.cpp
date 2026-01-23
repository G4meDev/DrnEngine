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
		Vector SocketOffset = Vector(2.25, -3.5f, 2.5);

		FrontLeftWheel.SocketLocation = SocketOffset * Vector(-1, 1, 1);
		FrontRightWheel.SocketLocation = SocketOffset * Vector(1, 1, 1);
		RearLeftWheel.SocketLocation = SocketOffset * Vector(-1, 1, -1);
		RearRightWheel.SocketLocation = SocketOffset * Vector(1, 1, -1);

		for (int32 i = 0; i < NUM_WHEELS; i++)
		{
			Wheels[i].Radius = 0.8f;
			Wheels[i].HalfWidth = 0.2f;
			Wheels[i].Mass = 10.0f;
			Wheels[i].DampingRate = 1.0f;

			Wheels[i].SusppensionLength = 0.4;
			Wheels[i].SusppensionStrength = 50;
			Wheels[i].SusppensionDamping = 15;
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

			for (int32 i = 0; i < NUM_WHEELS; i++)
			{
				PxVec3 v, w;
				PxF32 dist;
				PxVehicleComputeSuspensionRaycast(SimulationContext.frame, VehicleParams.wheelParams[i], VehicleParams.suspensionParams[i], VehicleState.steerCommandResponseStates[i], VehicleState.rigidBodyState.pose, v, w, dist);

				GetWorld()->DrawDebugLine(P2Vector(v), P2Vector(v) + P2Vector(w) * dist, Color::White, 0, 0);

				PxRaycastBuffer buff;
				GetWorld()->GetPhysicScene()->GetPhysxScene()->raycast( v, w, dist, buff, PxHitFlag::eDEFAULT );
				if(buff.hasBlock && buff.block.distance != 0.0f)
				{
					GetWorld()->DrawDebugSphere(P2Vector(buff.block.position), Quat::Identity, Color::Red, 0.2f, 20, 0, 0);
				}
			}

			ComponentSequence.update(DeltaTime, SimulationContext);
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

			// TODO: remove
			//const uint32 NumShapes = RigidActor->getNbShapes();
			//PxShape** Shapes = new PxShape*[NumShapes];
			//RigidActor->getShapes(Shapes, NumShapes);
			//for (int32 i = 0; i < NumShapes; i++)
			//{
			//	Shapes[i]->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
			//	Shapes[i]->setFlag(PxShapeFlag::eTRIGGER_SHAPE, true);
			//}
			//delete[] Shapes;

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
				wheelShape->setFlags(PxShapeFlag::eTRIGGER_SHAPE);
				//wheelShape->setSimulationFilterData(wheelShapeParams.simulationFilterData);
				//wheelShape->setQueryFilterData();

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

			bool bUseSweepCast = false;

			{
				VehicleParams.axleDescription.setToDefault();
				VehicleParams.suspensionStateCalculationParams.limitSuspensionExpansionVelocity = true;
				VehicleParams.suspensionStateCalculationParams.suspensionJounceCalculationType = bUseSweepCast ?
					PxVehicleSuspensionJounceCalculationType::eSWEEP : PxVehicleSuspensionJounceCalculationType::eRAYCAST;

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

					VehicleParams.suspensionParams[i].suspensionAttachment = PxTransform(Vector2P(Wheels[i].SocketLocation));
					VehicleParams.suspensionParams[i].suspensionTravelDir = PxVec3(0, -1, 0);
					VehicleParams.suspensionParams[i].suspensionTravelDist = Wheels[i].SusppensionLength;
					VehicleParams.suspensionParams[i].wheelAttachment = PxTransform(PxIdentity);

					VehicleParams.suspensionForceParams[i].damping = Wheels[i].SusppensionDamping;
					VehicleParams.suspensionForceParams[i].sprungMass = 1;
					VehicleParams.suspensionForceParams[i].stiffness = Wheels[i].SusppensionStrength;

					//static PxVec3 ForceOffset = PxVec3(0);
					//VehicleParams.suspensionComplianceParams[i].suspForceAppPoint.yVals = &ForceOffset;
				}

				PxVehicleConstraintsCreate(VehicleParams.axleDescription, *PhysicManager::Get()->GetPhysics(), *PhysxActor.rigidBody, VehicleState.physxConstraints);
			}

			{
				VehicleParams.physxRoadGeometryQueryParams.roadGeometryQueryType = bUseSweepCast ? PxVehiclePhysXRoadGeometryQueryType::eSWEEP : PxVehiclePhysXRoadGeometryQueryType::eRAYCAST;
				//VehicleParams.physxRoadGeometryQueryParams.roadGeometryQueryType = PxVehiclePhysXRoadGeometryQueryType::eSWEEP;
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

					VehicleParams.physxWheelShapeLocalPoses[wheelId] = PxTransform(PxIdentity);
					//VehicleParams.physxWheelShapeLocalPoses[wheelId] = PxTransform(Vector2P(Wheels[i].SocketLocation));
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
				ComponentSequence.add(static_cast<PxVehiclePhysXRoadGeometrySceneQueryComponent*>(&VehicleCommands));
				
				{
					ComponentSequenceSubstepGroupHandle = ComponentSequence.beginSubstepGroup(3);

					ComponentSequence.add(static_cast<PxVehicleSuspensionComponent*>(&VehicleCommands));
					//ComponentSequence.add(static_cast<PxVehicleTireComponent*>(this));
					ComponentSequence.add(static_cast<PxVehiclePhysXConstraintComponent*>(&VehicleCommands));
					//ComponentSequence.add(static_cast<PxVehicleEngineDrivetrainComponent*>(this));
					ComponentSequence.add(static_cast<PxVehicleRigidBodyComponent*>(&VehicleCommands));

					ComponentSequence.endSubstepGroup();
				}

				ComponentSequence.add(static_cast<PxVehicleWheelComponent*>(&VehicleCommands));

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

	void VehicleCommandsBase::getDataForSuspensionComponent(const PxVehicleAxleDescription*& axleDescription, const PxVehicleRigidBodyParams*& rigidBodyParams,
		const PxVehicleSuspensionStateCalculationParams*& suspensionStateCalculationParams, PxVehicleArrayData<const PxReal>& steerResponseStates,
		const PxVehicleRigidBodyState*& rigidBodyState, PxVehicleArrayData<const PxVehicleWheelParams>& wheelParams,
		PxVehicleArrayData<const PxVehicleSuspensionParams>& suspensionParams, PxVehicleArrayData<const PxVehicleSuspensionComplianceParams>& suspensionComplianceParams,
		PxVehicleArrayData<const PxVehicleSuspensionForceParams>& suspensionForceParams, PxVehicleSizedArrayData<const PxVehicleAntiRollForceParams>& antiRollForceParams,
		PxVehicleArrayData<const PxVehicleRoadGeometryState>& wheelRoadGeomStates, PxVehicleArrayData<PxVehicleSuspensionState>& suspensionStates,
		PxVehicleArrayData<PxVehicleSuspensionComplianceState>& suspensionComplianceStates, PxVehicleArrayData<PxVehicleSuspensionForce>& suspensionForces,
		PxVehicleAntiRollTorque*& antiRollTorque)
	{
		VehicleParams& Params = OwningVehicle->VehicleParams;
		VehicleState& State = OwningVehicle->VehicleState;

		axleDescription = &Params.axleDescription;
		rigidBodyParams = &Params.rigidBodyParams;
		suspensionStateCalculationParams = &Params.suspensionStateCalculationParams;
		steerResponseStates.setData(State.steerCommandResponseStates);
		rigidBodyState = &State.rigidBodyState;
		wheelParams.setData(Params.wheelParams);
		suspensionParams.setData(Params.suspensionParams);
		suspensionComplianceParams.setData(Params.suspensionComplianceParams);
		suspensionForceParams.setData(Params.suspensionForceParams);
		antiRollForceParams.setEmpty();
		wheelRoadGeomStates.setData(State.roadGeomStates);
		suspensionStates.setData(State.suspensionStates);
		suspensionComplianceStates.setData(State.suspensionComplianceStates);
		suspensionForces.setData(State.suspensionForces);
		antiRollTorque = NULL;
	}

	void VehicleCommandsBase::getDataForWheelComponent(const PxVehicleAxleDescription*& axleDescription,
		PxVehicleArrayData<const PxReal>& steerResponseStates, PxVehicleArrayData<const PxVehicleWheelParams>& wheelParams,
		PxVehicleArrayData<const PxVehicleSuspensionParams>& suspensionParams, PxVehicleArrayData<const PxVehicleWheelActuationState>& actuationStates,
		PxVehicleArrayData<const PxVehicleSuspensionState>& suspensionStates, PxVehicleArrayData<const PxVehicleSuspensionComplianceState>& suspensionComplianceStates,
		PxVehicleArrayData<const PxVehicleTireSpeedState>& tireSpeedStates, PxVehicleArrayData<PxVehicleWheelRigidBody1dState>& wheelRigidBody1dStates,
		PxVehicleArrayData<PxVehicleWheelLocalPose>& wheelLocalPoses)
	{
		VehicleParams& Params = OwningVehicle->VehicleParams;
		VehicleState& State = OwningVehicle->VehicleState;

		axleDescription = &Params.axleDescription;
		steerResponseStates.setData(State.steerCommandResponseStates);
		wheelParams.setData(Params.wheelParams);
		suspensionParams.setData(Params.suspensionParams);
		actuationStates.setData(State.actuationStates);
		suspensionStates.setData(State.suspensionStates);
		suspensionComplianceStates.setData(State.suspensionComplianceStates);
		tireSpeedStates.setData(State.tireSpeedStates);
		wheelRigidBody1dStates.setData(State.wheelRigidBody1dStates);
		wheelLocalPoses.setData(State.wheelLocalPoses);
	}

	void VehicleCommandsBase::getDataForPhysXConstraintComponent(const PxVehicleAxleDescription*& axleDescription, const PxVehicleRigidBodyState*& rigidBodyState,
		PxVehicleArrayData<const PxVehicleSuspensionParams>& suspensionParams, PxVehicleArrayData<const PxVehiclePhysXSuspensionLimitConstraintParams>& suspensionLimitParams,
		PxVehicleArrayData<const PxVehicleSuspensionState>& suspensionStates, PxVehicleArrayData<const PxVehicleSuspensionComplianceState>& suspensionComplianceStates,
		PxVehicleArrayData<const PxVehicleRoadGeometryState>& wheelRoadGeomStates, PxVehicleArrayData<const PxVehicleTireDirectionState>& tireDirectionStates,
		PxVehicleArrayData<const PxVehicleTireStickyState>& tireStickyStates, PxVehiclePhysXConstraints*& constraints)
	{
		VehicleParams& Params = OwningVehicle->VehicleParams;
		VehicleState& State = OwningVehicle->VehicleState;

		axleDescription = &Params.axleDescription;
		rigidBodyState = &State.rigidBodyState;
		suspensionParams.setData(Params.suspensionParams);
		suspensionLimitParams.setData(Params.physxSuspensionLimitConstraintParams);
		suspensionStates.setData(State.suspensionStates);
		suspensionComplianceStates.setData(State.suspensionComplianceStates);
		wheelRoadGeomStates.setData(State.roadGeomStates);
		tireDirectionStates.setData(State.tireDirectionStates);
		tireStickyStates.setData(State.tireStickyStates);
		constraints = &State.physxConstraints;
	}

	void VehicleCommandsBase::getDataForPhysXRoadGeometrySceneQueryComponent(const PxVehicleAxleDescription*& axleDescription, const PxVehiclePhysXRoadGeometryQueryParams*& roadGeomParams,
		PxVehicleArrayData<const PxReal>& steerResponseStates, const PxVehicleRigidBodyState*& rigidBodyState, PxVehicleArrayData<const PxVehicleWheelParams>& wheelParams,
		PxVehicleArrayData<const PxVehicleSuspensionParams>& suspensionParams, PxVehicleArrayData<const PxVehiclePhysXMaterialFrictionParams>& materialFrictionParams,
		PxVehicleArrayData<PxVehicleRoadGeometryState>& roadGeometryStates, PxVehicleArrayData<PxVehiclePhysXRoadGeometryQueryState>& physxRoadGeometryStates)
	{
		VehicleParams& Params = OwningVehicle->VehicleParams;
		VehicleState& State = OwningVehicle->VehicleState;

		axleDescription = &Params.axleDescription;
		roadGeomParams = &Params.physxRoadGeometryQueryParams;
		steerResponseStates.setData(State.steerCommandResponseStates);
		rigidBodyState = &State.rigidBodyState;
		wheelParams.setData(Params.wheelParams);
		suspensionParams.setData(Params.suspensionParams);
		materialFrictionParams.setData(Params.physxMaterialFrictionParams);
		roadGeometryStates.setData(State.roadGeomStates);
		physxRoadGeometryStates.setEmpty();
	}

        }  // namespace Drn