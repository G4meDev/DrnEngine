#include "DrnPCH.h"
#include "PhysicScene.h"
#include "PhysicManager.h"

LOG_DEFINE_CATEGORY( LogPhysicScene, "PhysicScene" )

namespace Drn
{


	PhysicScene::PhysicScene(World* InWorld)
		: m_OwningWorld(InWorld)
		, m_SimEventCallback(nullptr)
	{
		const physx::PxVec3 m_Gravity = physx::PxVec3( 0.0f, -9.81f, 0.0f );

		physx::PxSceneDesc sceneDesc(PhysicManager::Get()->GetToleranceScale());
		sceneDesc.gravity = m_Gravity;

		m_SimEventCallback = new PhysXSimEventCallback(this);

		physx::PxU32 numWorkers = 1;
		m_Dispatcher = physx::PxDefaultCpuDispatcherCreate(numWorkers);
		sceneDesc.cpuDispatcher	= m_Dispatcher;
		sceneDesc.simulationEventCallback = m_SimEventCallback;
		//sceneDesc.filterShader	= physx::PxDefaultSimulationFilterShader;
		sceneDesc.filterShader = contactReportFilterShader;

		sceneDesc.flags |= physx::PxSceneFlag::eENABLE_ACTIVE_ACTORS;
		m_PhysxScene = PhysicManager::Get()->GetPhysics()->createScene(sceneDesc);

#if WITH_EDITOR
		//m_PhysxScene->setVisualizationParameter( PxVisualizationParameter::eSCALE, 1 );
		//m_PhysxScene->setVisualizationParameter( PxVisualizationParameter::eCOLLISION_DYNAMIC, 1 );
		//m_PhysxScene->setVisualizationParameter( PxVisualizationParameter::eCOLLISION_STATIC, 1 );
		//m_PhysxScene->setVisualizationParameter( PxVisualizationParameter::eCOLLISION_SHAPES, 1 );
		//m_PhysxScene->setVisualizationParameter( PxVisualizationParameter::eCOLLISION_EDGES, 1 );
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
		if (m_SimEventCallback)
		{
			delete m_SimEventCallback;
			m_SimEventCallback = nullptr;
		}
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
			DispatchPhysicEvents();
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
				Transform WorldTransform = P2Transform( transform );
				Body->GetOwnerComponent()->SetWorldLocationAndRotation_SkipPhysic(WorldTransform.GetLocation(), WorldTransform.GetRotation());
			}
		}
	}

	void PhysicScene::DispatchPhysicEvents()
	{
		SCOPE_STAT( DispatchPhysicEvents );

		for (int32 i = 0; i < m_PendingCollisionNotifies.size(); i++)
		{
			CollisionNotifyInfo& NotifyInfo = m_PendingCollisionNotifies[i];
			if (NotifyInfo.RigidCollisionData.ContactInfos.size() > 0)
			{
				if (NotifyInfo.bCallEvent0 && NotifyInfo.IsValidForNotify() && NotifyInfo.Info0.m_Actor)
				{
					NotifyInfo.Info0.m_Actor->DispatchPhysicsCollisionHit(NotifyInfo.Info0, NotifyInfo.Info1, NotifyInfo.RigidCollisionData);
				}

				if (NotifyInfo.bCallEvent1 && NotifyInfo.IsValidForNotify() && NotifyInfo.Info1.m_Actor)
				{
					NotifyInfo.RigidCollisionData.SwapContactOrders();
					NotifyInfo.Info1.m_Actor->DispatchPhysicsCollisionHit(NotifyInfo.Info1, NotifyInfo.Info0, NotifyInfo.RigidCollisionData);
				}
			}
		}

		m_PendingCollisionNotifies.clear();
	}

	PxFilterFlags PhysicScene::contactReportFilterShader( PxFilterObjectAttributes attributes0, PxFilterData filterData0,
		PxFilterObjectAttributes attributes1, PxFilterData filterData1, PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize )
	{
		pairFlags = PxPairFlag::eSOLVE_CONTACT | PxPairFlag::eDETECT_DISCRETE_CONTACT
			| PxPairFlag::eNOTIFY_TOUCH_FOUND 
			| PxPairFlag::eNOTIFY_TOUCH_PERSISTS
			| PxPairFlag::eNOTIFY_CONTACT_POINTS;
		return PxFilterFlag::eDEFAULT;
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
		const Vector DebugDrawColor = Vector(0.3f, 0.7f, 0.2f);

		if (RigidActor)
		{
			Transform RigidTransform = P2Transform(RigidActor->getGlobalPose());

			const uint32 NumShapes = RigidActor->getNbShapes();
			PxShape** Shapes = new PxShape*[NumShapes];
			RigidActor->getShapes(Shapes, NumShapes);

			for (int32 i = 0; i < NumShapes; i++)
			{
				PxShape* Shape = Shapes[i];
				Transform LocalTransform = P2Transform( Shape->getLocalPose() );
				Transform WorldTransform = LocalTransform * RigidTransform;

				if ( Shape->getGeometry().getType() == PxGeometryType::eSPHERE)
				{
					const PxSphereGeometry* SphereGeo = static_cast<const PxSphereGeometry*>(&(Shape->getGeometry()));
					m_OwningWorld->DrawDebugSphere(WorldTransform.GetLocation(), WorldTransform.GetRotation(),
						DebugDrawColor, SphereGeo->radius, 16, 5.0f, 0);
				}

				else if ( Shape->getGeometry().getType() == PxGeometryType::eBOX)
				{
					const PxBoxGeometry* BoxGeo = static_cast<const PxBoxGeometry*>(&(Shape->getGeometry()));
					Box box = Box::BuildAABB(Vector::ZeroVector, P2Vector(BoxGeo->halfExtents));
					m_OwningWorld->DrawDebugBox(box, WorldTransform, DebugDrawColor, 5.0f, 0);
				}
			}

			delete Shapes;
		}
	}

	void PhysicScene::RaycastSingle( HitResult& Result, const Vector& Start, const Vector& Dir, float MaxDistance )
	{
		PxRaycastBuffer RaycastBuffer;
		bool Hit = m_PhysxScene->raycast( Vector2P( Start ), Vector2P(Dir), MaxDistance, RaycastBuffer);

		if (Hit)
		{
			BodyInstance* Body = PhysicUserData::Get<BodyInstance>( RaycastBuffer.block.actor->userData );
			PrimitiveComponent* HitPrimitive = Body ? Body->GetOwnerComponent() : nullptr;
			Actor* HitActor     = HitPrimitive ? HitPrimitive->GetOwningActor() : nullptr;

			if ( HitActor && !HitActor->IsMarkedPendingKill() )
			{
				Result = HitResult( Vector::ZeroVector, Vector::ZeroVector, P2Vector( RaycastBuffer.block.position ),
					P2Vector( RaycastBuffer.block.normal ), HitActor, HitPrimitive );
			}
		}
	}

	void PhysicScene::RaycastMulti( std::vector<HitResult>& Results, const Vector& Start, const Vector& Dir, float MaxDistance )
	{
		const PxU32 bufferSize = 256;
		PxRaycastHit hitBuffer[bufferSize];
		PxRaycastBuffer buf(hitBuffer, bufferSize);

		bool Hit = m_PhysxScene->raycast( Vector2P( Start ), Vector2P(Dir), MaxDistance, buf );
		Results.clear();
		Results.reserve( buf.getNbAnyHits() );

		if ( Hit )
		{
			for (uint32 i = 0; i < buf.getNbTouches(); i++)
			{
				BodyInstance* Body = PhysicUserData::Get<BodyInstance>(buf.touches[i].actor->userData);
				PrimitiveComponent* HitPrimitive = Body ? Body->GetOwnerComponent() : nullptr;
				Actor* HitActor = HitPrimitive ? HitPrimitive->GetOwningActor() : nullptr;

				if ( HitActor && !HitActor->IsMarkedPendingKill() )
				{
					Results.emplace_back( Vector::ZeroVector, Vector::ZeroVector, P2Vector( buf.touches[i].position ),
						P2Vector( buf.touches[i].normal ), HitActor, HitPrimitive );
				}
			}
		}
	}

// ------------------------------------------------------------------------------------------------------------

	void PhysXSimEventCallback::onContact( const PxContactPairHeader& PairHeader, const PxContactPair* Pairs, PxU32 NumPairs )
	{
		if ( PairHeader.flags & ( PxContactPairHeaderFlag::eREMOVED_ACTOR_0 | PxContactPairHeaderFlag::eREMOVED_ACTOR_1 ) )
		{
			LOG( LogPhysicScene, Info, TEXT( "onContact(): Actors have been deleted!" ) );
			return;
		}

		const PxActor* PActor0 = PairHeader.actors[0];
		const PxActor* PActor1 = PairHeader.actors[1];

		if (!PActor0 || !PActor1)
			__debugbreak();

		const PxRigidBody* PRigidBody0 = PActor0->is<PxRigidBody>();
		const PxRigidBody* PRigidBody1 = PActor1->is<PxRigidBody>();

		const BodyInstance* BodyInst0 = PhysicUserData::Get<BodyInstance>(PActor0->userData);
		const BodyInstance* BodyInst1 = PhysicUserData::Get<BodyInstance>(PActor1->userData);
	
		if(BodyInst0 == nullptr || BodyInst1 == nullptr || BodyInst0 == BodyInst1)
		{
			return;
		}

		AddCollisionNotifyInfo(BodyInst0, BodyInst1, Pairs, NumPairs, m_OwningScene->m_PendingCollisionNotifies);
	}

	bool CollisionNotifyInfo::IsValidForNotify() const
	{
		return (Info0.m_Component && Info1.m_Component);
	}

	void PhysXSimEventCallback::AddCollisionNotifyInfo( const BodyInstance* Body0, const BodyInstance* Body1,
		const physx::PxContactPair * Pairs, uint32 NumPairs, std::vector<CollisionNotifyInfo>& PendingNotifyInfos)
	{
		for(uint32 PairIdx = 0; PairIdx < NumPairs; ++PairIdx)
		{
			const PxContactPair* Pair = Pairs + PairIdx;

			if(!Pair->events.isSet(PxPairFlag::eNOTIFY_TOUCH_LOST) &&
				!Pair->events.isSet(PxPairFlag::eNOTIFY_THRESHOLD_FORCE_LOST) &&
				!Pair->flags.isSet(PxContactPairFlag::eREMOVED_SHAPE_0) &&
				!Pair->flags.isSet(PxContactPairFlag::eREMOVED_SHAPE_1))
			{
				const PxShape* Shape0 = Pair->shapes[0];
				if ( !Shape0 ) __debugbreak();
				const PxShape* Shape1 = Pair->shapes[1];
				if ( !Shape1 ) __debugbreak();

				CollisionNotifyInfo NotifyInfo;
				NotifyInfo.bCallEvent0 = true;
				NotifyInfo.Info0.SetFrom(Body0);
				NotifyInfo.bCallEvent1 = true;
				NotifyInfo.Info1.SetFrom(Body1);

				PxContactPairPoint ContactPointBuffer[16];
				int NumContactPoints = Pairs->extractContacts(ContactPointBuffer, 16);

				RigidBodyContactInfo ContactInfo;
				for (int i = 0; i < NumContactPoints; i++)
				{
					const PxContactPairPoint& Point = ContactPointBuffer[i];

					ContactInfo.ContactPosition = P2Vector( Point.position );
					ContactInfo.ContactNormal = P2Vector( Point.normal );

					NotifyInfo.RigidCollisionData.ContactInfos.push_back(ContactInfo);
				}

				PendingNotifyInfos.push_back(NotifyInfo);
			}
		}
	}

}