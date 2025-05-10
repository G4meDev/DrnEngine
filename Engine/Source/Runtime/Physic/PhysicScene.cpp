#include "DrnPCH.h"
#include "PhysicScene.h"
#include "PhysicManager.h"

LOG_DEFINE_CATEGORY( LogPhysicScene, "PhysicScene" )

namespace Drn
{
	PhysicScene::PhysicScene(World* InWorld)
		: m_OwningWorld(InWorld)
	{
		const physx::PxVec3 m_Gravity = physx::PxVec3( 0.0f, -9.81f, 0.0f );

		physx::PxSceneDesc sceneDesc(PhysicManager::Get()->GetToleranceScale());
		sceneDesc.gravity = m_Gravity;
		
		physx::PxU32 numWorkers = 1;
		m_Dispatcher = physx::PxDefaultCpuDispatcherCreate(numWorkers);
		sceneDesc.cpuDispatcher	= m_Dispatcher;
		sceneDesc.filterShader	= physx::PxDefaultSimulationFilterShader;

		sceneDesc.flags |= physx::PxSceneFlag::eENABLE_ACTIVE_ACTORS;

		m_PhysxScene = PhysicManager::Get()->GetPhysics()->createScene(sceneDesc);

#if WITH_EDITOR
		m_PhysxScene->setVisualizationParameter( PxVisualizationParameter::eSCALE, 1 );
		m_PhysxScene->setVisualizationParameter( PxVisualizationParameter::eCOLLISION_DYNAMIC, 1 );
		m_PhysxScene->setVisualizationParameter( PxVisualizationParameter::eCOLLISION_STATIC, 1 );
		m_PhysxScene->setVisualizationParameter( PxVisualizationParameter::eCOLLISION_SHAPES, 1 );
		m_PhysxScene->setVisualizationParameter( PxVisualizationParameter::eCOLLISION_EDGES, 1 );
#endif

#if WITH_PVD
		physx::PxPvdSceneClient* pvdClient = m_PhysxScene->getScenePvdClient();

		if ( pvdClient )
		{
			pvdClient->setScenePvdFlag( physx::PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, false );
			pvdClient->setScenePvdFlag( physx::PxPvdSceneFlag::eTRANSMIT_CONTACTS, true );
			pvdClient->setScenePvdFlag( physx::PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true );
		}
#endif
	}

	PhysicScene::~PhysicScene()
	{
		PX_RELEASE(m_PhysxScene);
		PX_RELEASE(m_Dispatcher);
	}

	void PhysicScene::Release()
	{
		delete this;
	}

	void PhysicScene::Tick( float DeltaTime )
	{
		SCOPE_STAT(PhysicSceneTick);

		if (IsSimulating())
		{
			StepSimulation(DeltaTime);
			SyncActors();
		}

		if (m_DrawDebugCollision)
		{
			DrawDebugCollisions();
		}
	}

	bool PhysicScene::IsSimulating() const
	{
		return m_PhysxScene && m_OwningWorld && m_OwningWorld->IsTicking();
	}

	void PhysicScene::AddActor( physx::PxActor* InActor )
	{
		if (m_PhysxScene)
		{
			m_PhysxScene->addActor(*InActor);
		}

		else
		{
			LOG(LogPhysicScene, Error, "tring to add actor to non existing physic scene.")
		}
	}

	void PhysicScene::RemoveActor( physx::PxActor* InActor )
	{
		if (m_PhysxScene)
		{
			m_PhysxScene->removeActor(*InActor);
		}

		else
		{
			LOG(LogPhysicScene, Error, "tring to remove actor from non existing physic scene.")
		}
	}

	void PhysicScene::StepSimulation( float DeltaTime )
	{
		m_PhysxScene->lockWrite();
		SCOPE_STAT(StepSimulation);

		m_PhysxScene->simulate(DeltaTime);
		m_PhysxScene->fetchResults(true);

		m_PhysxScene->unlockWrite();
	}

	void PhysicScene::SyncActors()
	{
		SCOPE_STAT(SyncActors);

		physx::PxU32 ActorCount = 0;
		physx::PxActor** ActiveActors = m_PhysxScene->getActiveActors(ActorCount);

		for (physx::PxU32 i = 0; i < ActorCount; i++)
		{
			physx::PxActor* ActiveActor = ActiveActors[i];
			physx::PxRigidActor* RigidActor = ActiveActor->is<physx::PxRigidActor>();

			if (BodyInstance* Body = PhysicUserData::Get<BodyInstance>(RigidActor->userData))
			{
				physx::PxTransform transform(RigidActor->getGlobalPose());
				Body->GetOwnerComponent()->SetWorldTransform(P2Transform(transform));
			}
		}
	}

// ------------------------------------------------------------------------------------------------------------

	void PhysicScene::DrawDebugCollisions()
	{
		SCOPE_STAT(DebugDraw);

		//const PxRenderBuffer& Rb = GetPhysxScene()->getRenderBuffer();
		//uint32 NumLines = Rb.getNbLines();
		//
		//for ( PxU32 i = 0; i < NumLines; i++ )
		//{
		//	const PxDebugLine& line = Rb.getLines()[i];
		//	m_OwningWorld->DrawDebugLine(P2Vector(line.pos0), P2Vector(line.pos1), Vector::FromU32(line.color0), 0);
		//}

// ------------------------------------------------------------------------------------------------------------

		PxActorTypeFlags Flags = PxActorTypeFlag::eRIGID_STATIC | PxActorTypeFlag::eRIGID_DYNAMIC;
	
		uint32 NumActors = GetPhysxScene()->getNbActors(Flags);
		PxActor** Actors = new PxActor*[NumActors];

		uint32 ActorsNum = GetPhysxScene()->getActors(Flags, Actors, NumActors);

		for (int32 i = 0; i < ActorsNum; i++)
		{
			PxActor* Actor = Actors[i];
				
			if ( Actor && Actor->getOwnerClient() == PX_DEFAULT_CLIENT )
			{
				PxRigidActor* RigidActor = Actor->is<PxRigidActor>();
				DrawDebugForRigidActor(RigidActor);

				BodyInstance* Body = PhysicUserData::Get<BodyInstance>(RigidActor->userData);
			}
		}

		delete Actors;
	}

	void PhysicScene::DrawDebugForRigidActor( PxRigidActor* RigidActor )
	{
		if (RigidActor)
		{
			Transform RigidTransform = P2Transform(RigidActor->getGlobalPose());

			const uint32 NumShapes = RigidActor->getNbShapes();
			PxShape** Shapes = new PxShape*[NumShapes];
			RigidActor->getShapes(Shapes, NumShapes);

			for (int32 i = 0; i < NumShapes; i++)
			{
				PxShape* Shape = Shapes[i];

				if ( Shape->getGeometry().getType() == PxGeometryType::eSPHERE)
				{
					const PxSphereGeometry* SphereGeo = static_cast<const PxSphereGeometry*>(&(Shape->getGeometry()));
					m_OwningWorld->DrawDebugSphere(RigidTransform.GetLocation(), RigidTransform.GetRotation(),
						Vector::ZeroVector, SphereGeo->radius, 16, 5.0f, 0);
				}

				else if ( Shape->getGeometry().getType() == PxGeometryType::eBOX)
				{
					const PxBoxGeometry* BoxGeo = static_cast<const PxBoxGeometry*>(&(Shape->getGeometry()));
					Box box = Box::BuildAABB(Vector::ZeroVector, P2Vector(BoxGeo->halfExtents));
					m_OwningWorld->DrawDebugBox(box, RigidTransform, Vector(0.3f, 0.7f, 0.2f), 5.0f, 0);
				}
			}

			delete Shapes;
		}
	}

}