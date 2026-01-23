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
		TransmissionCommandState.setToDefault();

		// TODO: remove
		gPhysXMaterialFrictions[0].friction = 1.0f;
		gPhysXMaterialFrictions[0].material = PhysicManager::Get()->TempMaterial;

		//Vector SocketOffset = Vector(2.25, 0.98f, 2.5);
		Vector SocketOffset = Vector(2.25, -3.1f, 2.5);

		FrontLeftWheel.SocketLocation = SocketOffset * Vector(-1, 1, 1);
		FrontRightWheel.SocketLocation = SocketOffset * Vector(1, 1, 1);
		RearLeftWheel.SocketLocation = SocketOffset * Vector(-1, 1, -1);
		RearRightWheel.SocketLocation = SocketOffset * Vector(1, 1, -1);

		for (int32 i = 0; i < NUM_WHEELS; i++)
		{
			Wheels[i].Radius = 0.8f;
			Wheels[i].HalfWidth = 0.2f;
			Wheels[i].Mass = 20.0f;
			Wheels[i].DampingRate = 0.25f;

			Wheels[i].SusppensionLength = 0.5;
			Wheels[i].SusppensionStrength = 35000;
			Wheels[i].SusppensionDamping = 8000;
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

			CommandState.throttle = ThrottleInput;
			CommandState.steer = SteerInput;
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
				VehicleParams.rigidBodyParams.moi = RigidBody->getMassSpaceInertiaTensor();

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
					VehicleParams.suspensionForceParams[i].sprungMass = VehicleParams.rigidBodyParams.mass / 4;
					VehicleParams.suspensionForceParams[i].stiffness = Wheels[i].SusppensionStrength;

					PxVec3 ForceOffset = PxVec3(0);

					VehicleParams.suspensionComplianceParams[i].suspForceAppPoint.clear();
					VehicleParams.suspensionComplianceParams[i].suspForceAppPoint.addPair(0, ForceOffset);

					VehicleParams.suspensionComplianceParams[i].tireForceAppPoint.clear();
					VehicleParams.suspensionComplianceParams[i].tireForceAppPoint.addPair(0, ForceOffset);

					VehicleParams.suspensionComplianceParams[i].wheelCamberAngle.clear();
					VehicleParams.suspensionComplianceParams[i].wheelCamberAngle.addPair(0, 0);

					VehicleParams.suspensionComplianceParams[i].wheelToeAngle.clear();
					VehicleParams.suspensionComplianceParams[i].wheelToeAngle.addPair(0, 0);

					{
						VehicleParams.tireForceParams[0].longStiff = 25000.0f;
						VehicleParams.tireForceParams[0].latStiffX = 0.01f;
						VehicleParams.tireForceParams[0].latStiffY = 120000.0f;
						VehicleParams.tireForceParams[0].camberStiff = 0.0f;
						VehicleParams.tireForceParams[0].restLoad = 5500.0f;

						VehicleParams.tireForceParams[0].frictionVsSlip[0][0] = 0.0f;
						VehicleParams.tireForceParams[0].frictionVsSlip[0][1] = 1.0f;
						VehicleParams.tireForceParams[0].frictionVsSlip[1][0] = 0.1f;
						VehicleParams.tireForceParams[0].frictionVsSlip[1][1] = 1.0f;
						VehicleParams.tireForceParams[0].frictionVsSlip[2][0] = 1.0f;
						VehicleParams.tireForceParams[0].frictionVsSlip[2][1] = 1.0f;

						VehicleParams.tireForceParams[0].loadFilter[0][0] = 0.0f;
						VehicleParams.tireForceParams[0].loadFilter[0][1] = 0.23f;
						VehicleParams.tireForceParams[0].loadFilter[1][0] = 3.0f;
						VehicleParams.tireForceParams[0].loadFilter[1][1] = 3.0f;

					}
					{
						VehicleParams.tireForceParams[1].longStiff = 25000.0f;
						VehicleParams.tireForceParams[1].latStiffX = 0.01f;
						VehicleParams.tireForceParams[1].latStiffY = 120000.0f;
						VehicleParams.tireForceParams[1].camberStiff = 0.0f;
						VehicleParams.tireForceParams[1].restLoad = 5500.0f;

						VehicleParams.tireForceParams[1].frictionVsSlip[0][0] = 0.0f;
						VehicleParams.tireForceParams[1].frictionVsSlip[0][1] = 1.0f;
						VehicleParams.tireForceParams[1].frictionVsSlip[1][0] = 0.1f;
						VehicleParams.tireForceParams[1].frictionVsSlip[1][1] = 1.0f;
						VehicleParams.tireForceParams[1].frictionVsSlip[2][0] = 1.0f;
						VehicleParams.tireForceParams[1].frictionVsSlip[2][1] = 1.0f;

						VehicleParams.tireForceParams[1].loadFilter[0][0] = 0.0f;
						VehicleParams.tireForceParams[1].loadFilter[0][1] = 0.23f;
						VehicleParams.tireForceParams[1].loadFilter[1][0] = 3.0f;
						VehicleParams.tireForceParams[1].loadFilter[1][1] = 3.0f;
					}
										{
						VehicleParams.tireForceParams[2].longStiff = 25000.0f;
						VehicleParams.tireForceParams[2].latStiffX = 0.01f;
						VehicleParams.tireForceParams[2].latStiffY = 150000.0f;
						VehicleParams.tireForceParams[2].camberStiff = 0.0f;
						VehicleParams.tireForceParams[2].restLoad = 4500.0f;

						VehicleParams.tireForceParams[2].frictionVsSlip[0][0] = 0.0f;
						VehicleParams.tireForceParams[2].frictionVsSlip[0][1] = 1.0f;
						VehicleParams.tireForceParams[2].frictionVsSlip[1][0] = 0.1f;
						VehicleParams.tireForceParams[2].frictionVsSlip[1][1] = 1.0f;
						VehicleParams.tireForceParams[2].frictionVsSlip[2][0] = 1.0f;
						VehicleParams.tireForceParams[2].frictionVsSlip[2][1] = 1.0f;

						VehicleParams.tireForceParams[2].loadFilter[0][0] = 0.0f;
						VehicleParams.tireForceParams[2].loadFilter[0][1] = 0.23f;
						VehicleParams.tireForceParams[2].loadFilter[1][0] = 3.0f;
						VehicleParams.tireForceParams[2].loadFilter[1][1] = 3.0f;
					}
					{
						VehicleParams.tireForceParams[3].longStiff = 25000.0f;
						VehicleParams.tireForceParams[3].latStiffX = 0.01f;
						VehicleParams.tireForceParams[3].latStiffY = 150000.0f;
						VehicleParams.tireForceParams[3].camberStiff = 0.0f;
						VehicleParams.tireForceParams[3].restLoad = 4500.0f;

						VehicleParams.tireForceParams[3].frictionVsSlip[0][0] = 0.0f;
						VehicleParams.tireForceParams[3].frictionVsSlip[0][1] = 1.0f;
						VehicleParams.tireForceParams[3].frictionVsSlip[1][0] = 0.1f;
						VehicleParams.tireForceParams[3].frictionVsSlip[1][1] = 1.0f;
						VehicleParams.tireForceParams[3].frictionVsSlip[2][0] = 1.0f;
						VehicleParams.tireForceParams[3].frictionVsSlip[2][1] = 1.0f;

						VehicleParams.tireForceParams[3].loadFilter[0][0] = 0.0f;
						VehicleParams.tireForceParams[3].loadFilter[0][1] = 0.23f;
						VehicleParams.tireForceParams[3].loadFilter[1][0] = 3.0f;
						VehicleParams.tireForceParams[3].loadFilter[1][1] = 3.0f;
					}

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
				}
				
				VehicleParams.physxActorCMassLocalPose = RigidBody->getCMassLocalPose();
				//VehicleParams.physxActorBoxShapeHalfExtents = actorBoxShapeHalfExtents;
				//VehicleParams.physxActorBoxShapeLocalPose = actorBoxShapeLocalPose;
			}

			{
				VehicleParams.brakeResponseParams[0].maxResponse = 1875.0f;
				VehicleParams.brakeResponseParams[0].wheelResponseMultipliers[0] = 1.0f;
				VehicleParams.brakeResponseParams[0].wheelResponseMultipliers[1] = 1.0f;
				VehicleParams.brakeResponseParams[0].wheelResponseMultipliers[2] = 1.0f;
				VehicleParams.brakeResponseParams[0].wheelResponseMultipliers[3] = 1.0f;

				// handbrake
				VehicleParams.brakeResponseParams[1].maxResponse = 0.0f;
				VehicleParams.brakeResponseParams[1].wheelResponseMultipliers[0] = 0.0f;
				VehicleParams.brakeResponseParams[1].wheelResponseMultipliers[1] = 0.0f;
				VehicleParams.brakeResponseParams[1].wheelResponseMultipliers[2] = 1.0f;
				VehicleParams.brakeResponseParams[1].wheelResponseMultipliers[3] = 1.0f;

				VehicleParams.steerResponseParams.maxResponse = 0.5f;
				VehicleParams.steerResponseParams.wheelResponseMultipliers[0] = 1.0f;
				VehicleParams.steerResponseParams.wheelResponseMultipliers[1] = 1.0f;
				VehicleParams.steerResponseParams.wheelResponseMultipliers[2] = 0.0f;
				VehicleParams.steerResponseParams.wheelResponseMultipliers[3] = 0.0f;

				VehicleParams.ackermannParams[0].wheelIds[0] = 0;
				VehicleParams.ackermannParams[0].wheelIds[1] = 1;
				// ???
				VehicleParams.ackermannParams[0].wheelBase = Wheels[1].SocketLocation.GetZ() * 2;
				VehicleParams.ackermannParams[0].trackWidth = Wheels[1].SocketLocation.GetX() * 2;
				VehicleParams.ackermannParams[0].strength = 1.0f;

				VehicleParams.autoboxParams.upRatios[0] = 0.65f;
				VehicleParams.autoboxParams.upRatios[1] = 0.15f;
				VehicleParams.autoboxParams.upRatios[2] = 0.65f;
				VehicleParams.autoboxParams.upRatios[3] = 0.65f;
				VehicleParams.autoboxParams.upRatios[4] = 0.65f;
				VehicleParams.autoboxParams.upRatios[5] = 0.65f;
				VehicleParams.autoboxParams.upRatios[6] = 0.65f;

				VehicleParams.autoboxParams.downRatios[0] = 0.5f;
				VehicleParams.autoboxParams.downRatios[1] = 0.5f;
				VehicleParams.autoboxParams.downRatios[2] = 0.5f;
				VehicleParams.autoboxParams.downRatios[3] = 0.5f;
				VehicleParams.autoboxParams.downRatios[4] = 0.5f;
				VehicleParams.autoboxParams.downRatios[5] = 0.5f;
				VehicleParams.autoboxParams.downRatios[6] = 0.5f;
				
				VehicleParams.autoboxParams.latency = 2.0f;

				VehicleParams.clutchCommandResponseParams.maxResponse = 10.0f;

				VehicleParams.engineParams.torqueCurve.clear();
				VehicleParams.engineParams.torqueCurve.addPair(0.0f, 1.0f);
				VehicleParams.engineParams.torqueCurve.addPair(0.33f, 1.0f);
				VehicleParams.engineParams.torqueCurve.addPair(1.0f, 1.0f);

				VehicleParams.engineParams.moi = 1.0;
				VehicleParams.engineParams.peakTorque = 500.0;
				VehicleParams.engineParams.idleOmega = 0.0;
				VehicleParams.engineParams.maxOmega = 600.0;
				VehicleParams.engineParams.dampingRateFullThrottle = 0.15;
				VehicleParams.engineParams.dampingRateZeroThrottleClutchEngaged = 2.0;
				VehicleParams.engineParams.dampingRateZeroThrottleClutchDisengaged = 0.35;

				VehicleParams.gearBoxParams.neutralGear = 1;
				VehicleParams.gearBoxParams.finalRatio = 4.0f;
				VehicleParams.gearBoxParams.switchTime = 0.5f;
				VehicleParams.gearBoxParams.ratios[0] = -4.0f;
				VehicleParams.gearBoxParams.ratios[1] = 0.0f;
				VehicleParams.gearBoxParams.ratios[2] = 4.0f;
				VehicleParams.gearBoxParams.ratios[3] = 2.0f;
				VehicleParams.gearBoxParams.ratios[4] = 1.5f;
				VehicleParams.gearBoxParams.ratios[5] = 1.1f;
				VehicleParams.gearBoxParams.ratios[6] = 1.0f;

				VehicleParams.fourWheelDifferentialParams.torqueRatios[0] = 0.25f;
				VehicleParams.fourWheelDifferentialParams.torqueRatios[1] = 0.25f;
				VehicleParams.fourWheelDifferentialParams.torqueRatios[2] = 0.25f;
				VehicleParams.fourWheelDifferentialParams.torqueRatios[3] = 0.25f;

				VehicleParams.fourWheelDifferentialParams.aveWheelSpeedRatios[0] = 0.25f;
				VehicleParams.fourWheelDifferentialParams.aveWheelSpeedRatios[1] = 0.25f;
				VehicleParams.fourWheelDifferentialParams.aveWheelSpeedRatios[2] = 0.25f;
				VehicleParams.fourWheelDifferentialParams.aveWheelSpeedRatios[3] = 0.25f;

				VehicleParams.fourWheelDifferentialParams.frontWheelIds[0] = 0;
				VehicleParams.fourWheelDifferentialParams.frontWheelIds[1] = 1;
				VehicleParams.fourWheelDifferentialParams.rearWheelIds[0] = 2;
				VehicleParams.fourWheelDifferentialParams.rearWheelIds[1] = 3;

				VehicleParams.fourWheelDifferentialParams.centerBias = 1.3f;
				VehicleParams.fourWheelDifferentialParams.centerTarget = 1.29f;
				VehicleParams.fourWheelDifferentialParams.frontBias = 1.3f;
				VehicleParams.fourWheelDifferentialParams.frontTarget = 1.29f;
				VehicleParams.fourWheelDifferentialParams.rearBias = 1.3f;
				VehicleParams.fourWheelDifferentialParams.rearTarget = 1.29f;
				VehicleParams.fourWheelDifferentialParams.rate = 10.0f;
			}

			{
				ComponentSequence.add(static_cast<PxVehiclePhysXActorBeginComponent*>(&VehicleCommands));

				ComponentSequence.add(static_cast<PxVehicleEngineDriveCommandResponseComponent*>(&VehicleCommands));
				ComponentSequence.add(static_cast<PxVehicleFourWheelDriveDifferentialStateComponent*>(&VehicleCommands));
				ComponentSequence.add(static_cast<PxVehicleEngineDriveActuationStateComponent*>(&VehicleCommands));
				ComponentSequence.add(static_cast<PxVehiclePhysXRoadGeometrySceneQueryComponent*>(&VehicleCommands));
				
				{
					ComponentSequenceSubstepGroupHandle = ComponentSequence.beginSubstepGroup(3);

					ComponentSequence.add(static_cast<PxVehicleSuspensionComponent*>(&VehicleCommands));
					ComponentSequence.add(static_cast<PxVehicleTireComponent*>(&VehicleCommands));
					ComponentSequence.add(static_cast<PxVehiclePhysXConstraintComponent*>(&VehicleCommands));
					ComponentSequence.add(static_cast<PxVehicleEngineDrivetrainComponent*>(&VehicleCommands));
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
		VehicleParams& Params = OwningVehicle->VehicleParams;
		VehicleState& State = OwningVehicle->VehicleState;

		axleDescription = &Params.axleDescription;
		brakeResponseParams.setDataAndCount(Params.brakeResponseParams, sizeof(Params.brakeResponseParams) / sizeof(PxVehicleBrakeCommandResponseParams));
		steerResponseParams = &Params.steerResponseParams;
		ackermannParams.setDataAndCount(Params.ackermannParams, sizeof(Params.ackermannParams)/sizeof(PxVehicleAckermannParams));
		gearboxParams = &Params.gearBoxParams;
		clutchResponseParams = &Params.clutchCommandResponseParams;
		engineParams = &Params.engineParams;
		rigidBodyState = &State.rigidBodyState;
		engineState = &State.engineState;
		autoboxParams = &Params.autoboxParams;
		commands = &OwningVehicle->CommandState;
		transmissionCommands = &OwningVehicle->TransmissionCommandState;
		brakeResponseStates.setData(State.brakeCommandResponseStates);
		throttleResponseState = &State.throttleCommandResponseState;
		steerResponseStates.setData(State.steerCommandResponseStates);
		gearboxResponseState = &State.gearboxState;
		clutchResponseState = &State.clutchCommandResponseState;
		autoboxState = &State.autoboxState;
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

	void VehicleCommandsBase::getDataForFourWheelDriveDifferentialStateComponent(const PxVehicleAxleDescription*& axleDescription, const PxVehicleFourWheelDriveDifferentialParams*& differentialParams,
		PxVehicleArrayData<const PxVehicleWheelRigidBody1dState>& wheelRigidbody1dStates, PxVehicleDifferentialState*& differentialState,
		PxVehicleWheelConstraintGroupState*& wheelConstraintGroupState)
	{
		VehicleParams& Params = OwningVehicle->VehicleParams;
		VehicleState& State = OwningVehicle->VehicleState;

		axleDescription = &Params.axleDescription;
		differentialParams = &Params.fourWheelDifferentialParams;
		wheelRigidbody1dStates.setData(State.wheelRigidBody1dStates);
		differentialState = &State.differentialState;
		wheelConstraintGroupState = &State.wheelConstraintGroupState;
	}

	void VehicleCommandsBase::getDataForEngineDrivetrainComponent(const PxVehicleAxleDescription*& axleDescription, PxVehicleArrayData<const PxVehicleWheelParams>& wheelParams,
		const PxVehicleEngineParams*& engineParams, const PxVehicleClutchParams*& clutchParams, const PxVehicleGearboxParams*& gearboxParams, PxVehicleArrayData<const PxReal>& brakeResponseStates,
		PxVehicleArrayData<const PxVehicleWheelActuationState>& actuationStates, PxVehicleArrayData<const PxVehicleTireForce>& tireForces,
		const PxVehicleEngineDriveThrottleCommandResponseState*& throttleResponseState, const PxVehicleClutchCommandResponseState*& clutchResponseState,
		const PxVehicleDifferentialState*& differentialState, const PxVehicleWheelConstraintGroupState*& constraintGroupState,
		PxVehicleArrayData<PxVehicleWheelRigidBody1dState>& wheelRigidBody1dStates, PxVehicleEngineState*& engineState, PxVehicleGearboxState*& gearboxState,
		PxVehicleClutchSlipState*& clutchState)
	{
		VehicleParams& Params = OwningVehicle->VehicleParams;
		VehicleState& State = OwningVehicle->VehicleState;

		axleDescription = &Params.axleDescription;
		wheelParams.setData(Params.wheelParams);
		engineParams = &Params.engineParams;
		clutchParams = &Params.clutchParams;
		gearboxParams = &Params.gearBoxParams;
		brakeResponseStates.setData(State.brakeCommandResponseStates);
		actuationStates.setData(State.actuationStates);
		tireForces.setData(State.tireForces);
		throttleResponseState = &State.throttleCommandResponseState;
		clutchResponseState = &State.clutchCommandResponseState;
		differentialState = &State.differentialState;
		constraintGroupState = NULL;
		wheelRigidBody1dStates.setData(State.wheelRigidBody1dStates);
		engineState = &State.engineState;
		gearboxState = &State.gearboxState;
		clutchState = &State.clutchState;
	}

	void VehicleCommandsBase::getDataForTireComponent(const PxVehicleAxleDescription*& axleDescription, PxVehicleArrayData<const PxReal>& steerResponseStates,
		const PxVehicleRigidBodyState*& rigidBodyState, PxVehicleArrayData<const PxVehicleWheelActuationState>& actuationStates,
		PxVehicleArrayData<const PxVehicleWheelParams>& wheelParams, PxVehicleArrayData<const PxVehicleSuspensionParams>& suspensionParams,
		PxVehicleArrayData<const PxVehicleTireForceParams>& tireForceParams, PxVehicleArrayData<const PxVehicleRoadGeometryState>& roadGeomStates,
		PxVehicleArrayData<const PxVehicleSuspensionState>& suspensionStates, PxVehicleArrayData<const PxVehicleSuspensionComplianceState>& suspensionComplianceStates,
		PxVehicleArrayData<const PxVehicleSuspensionForce>& suspensionForces, PxVehicleArrayData<const PxVehicleWheelRigidBody1dState>& wheelRigidBody1DStates,
		PxVehicleArrayData<PxVehicleTireGripState>& tireGripStates, PxVehicleArrayData<PxVehicleTireDirectionState>& tireDirectionStates,
		PxVehicleArrayData<PxVehicleTireSpeedState>& tireSpeedStates, PxVehicleArrayData<PxVehicleTireSlipState>& tireSlipStates,
		PxVehicleArrayData<PxVehicleTireCamberAngleState>& tireCamberAngleStates, PxVehicleArrayData<PxVehicleTireStickyState>& tireStickyStates,
		PxVehicleArrayData<PxVehicleTireForce>& tireForces)
	{
		VehicleParams& Params = OwningVehicle->VehicleParams;
		VehicleState& State = OwningVehicle->VehicleState;

		axleDescription = &Params.axleDescription;
		steerResponseStates.setData(State.steerCommandResponseStates);
		rigidBodyState = &State.rigidBodyState;
		actuationStates.setData(State.actuationStates);
		wheelParams.setData(Params.wheelParams);
		suspensionParams.setData(Params.suspensionParams);
		tireForceParams.setData(Params.tireForceParams);
		roadGeomStates.setData(State.roadGeomStates);
		suspensionStates.setData(State.suspensionStates);
		suspensionComplianceStates.setData(State.suspensionComplianceStates);
		suspensionForces.setData(State.suspensionForces);
		wheelRigidBody1DStates.setData(State.wheelRigidBody1dStates);
		tireGripStates.setData(State.tireGripStates);
		tireDirectionStates.setData(State.tireDirectionStates);
		tireSpeedStates.setData(State.tireSpeedStates);
		tireSlipStates.setData(State.tireSlipStates);
		tireCamberAngleStates.setData(State.tireCamberAngleStates);
		tireStickyStates.setData(State.tireStickyStates);
		tireForces.setData(State.tireForces);
	}

	void VehicleCommandsBase::getDataForEngineDriveActuationStateComponent(const PxVehicleAxleDescription*& axleDescription, const PxVehicleGearboxParams*& gearboxParams,
		PxVehicleArrayData<const PxReal>& brakeResponseStates, const PxVehicleEngineDriveThrottleCommandResponseState*& throttleResponseState,
		const PxVehicleGearboxState*& gearboxState, const PxVehicleDifferentialState*& differentialState,
		const PxVehicleClutchCommandResponseState*& clutchResponseState, PxVehicleArrayData<PxVehicleWheelActuationState>& actuationStates)
	{
		VehicleParams& Params = OwningVehicle->VehicleParams;
		VehicleState& State = OwningVehicle->VehicleState;

		axleDescription = &Params.axleDescription;
		gearboxParams = &Params.gearBoxParams;
		brakeResponseStates.setData(State.brakeCommandResponseStates);
		throttleResponseState = &State.throttleCommandResponseState;
		gearboxState = &State.gearboxState;
		differentialState = &State.differentialState;
		clutchResponseState = &State.clutchCommandResponseState;
		actuationStates.setData(State.actuationStates);
	}

        }  // namespace Drn