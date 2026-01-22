#include "DrnPCH.h"
#include "WheeledVehicleMovementComponent.h"

namespace Drn
{
	WheeledVehicleMovementComponent::WheeledVehicleMovementComponent()
		: Component()
		, OwningVehicle(nullptr)
		, RigidBody(nullptr)
		, SteerInput(0.0f)
		, ThrottleInput(0.0f)
	{
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
		}

		//physxActor.rigidBody = nullptr;

		//BaseState.setToDefault();
		//CommandState.setToDefault();
		//TransmissionCommandState.setToDefault();
		//
		//ComponentSequence.setSubsteps(ComponentSequenceSubstepGroupHandle, 3);

	}

	WheeledVehicleMovementComponent::~WheeledVehicleMovementComponent()
	{
		
	}

	void WheeledVehicleMovementComponent::Tick( float DeltaTime )
	{
		Component::Tick(DeltaTime);

		drn_check(OwningVehicle);

		//ComponentSequence.update(DeltaTime, );

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

		//PxVehicleFrame Frame;
		//Frame.setToDefault();
		//
		//PxVehicleRigidBodyParams RigidBodyParams;
		//RigidBodyParams.mass = 1000.0f;
		//PxVehiclePhysXRigidActorParams physxActorParams(RigidBodyParams, NULL);
		//
		//PxTransform ComLocal;
		//
		//const PxBoxGeometry boxGeom(2, 3, 4);
		//PxTransform boxLocal;
		//const PxVehiclePhysXRigidActorShapeParams physxActorShapeParams(boxGeom, boxLocal, *PhysicManager::Get()->TempMaterial, PxShapeFlags(0), PxFilterData(), PxFilterData());
		//
		//PxVehicleAxleDescription axleDescription;
		//axleDescription.setToDefault();
		//
		//PxVehicleWheelParams wheelParams[4];
		//for (int32 i = 0; i < 4; i++)
		//{
		//	wheelParams[i].dampingRate = 150;
		//	wheelParams[i].halfWidth = 0.4;
		//	wheelParams[i].radius = 0.8;
		//	wheelParams[i].mass = 10.0;
		//	wheelParams[i].moi = 1.0f;
		//}
		//
		//const PxVehiclePhysXWheelParams physxWheelParams(axleDescription, wheelParams);
		//const PxVehiclePhysXWheelShapeParams physxWheelShapeParams(*PhysicManager::Get()->TempMaterial, PxShapeFlags(0), PxFilterData(), PxFilterData());
		//
		//PxCookingParams CookParams = PxCookingParams(physx::PxTolerancesScale());
		//
		//PxVehiclePhysXActorCreate(Frame, physxActorParams, ComLocal, physxActorShapeParams, physxWheelParams, physxWheelShapeParams, *PhysicManager::Get()->GetPhysics(), CookParams, physxActor);
		//
		//GetWorld()->GetPhysicScene()->AddActor(physxActor.rigidBody);

		if (PxRigidActor* RigidActor = OwningVehicle->GetVehicleBody()->GetBodyInstance().GetRigidActor())
		{
			if (RigidBody = RigidActor->is<PxRigidDynamic>())
			{
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

					//vehiclePhysXActor.wheelShapes[wheelId] = wheelShape;
				}
			}
		}
		//RigidBody = physics.createRigidDynamic(PxTransform(PxIdentity));
		//vehiclePhysXActor.rigidBody = rd;
		//PxVehiclePhysXActorConfigure(rigidActorParams, rigidActorCmassLocalPose, *rd);
		//createShapes(vehicleFrame, rigidActorShapeParams, wheelParams, wheelShapeParams, rd, physics, params, vehiclePhysXActor);
	}

	void WheeledVehicleMovementComponent::UnRegisterComponent()
	{
		//if (physxActor.rigidBody)
		//{
		//	PxVehiclePhysXActorDestroy(physxActor);
		//}

		Component::UnRegisterComponent();
	}


#if WITH_EDITOR
	void WheeledVehicleMovementComponent::DrawDetailPanel( float DeltaTime )
	{
		Component::DrawDetailPanel(DeltaTime);

	}

#endif

}  // namespace Drn