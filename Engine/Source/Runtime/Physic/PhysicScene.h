#pragma once

#include "ForwardTypes.h"

LOG_DECLARE_CATEGORY( LogPhysicScene )

using namespace physx;

namespace Drn
{
	class PhysXSimEventCallback : public PxSimulationEventCallback
	{
	public:
		PhysXSimEventCallback(PhysicScene* InOwningScene)
			: m_OwningScene(InOwningScene)
		{
		}

		virtual void onConstraintBreak(PxConstraintInfo* constraints, PxU32 count) override {};
		virtual void onWake(PxActor** actors, PxU32 count) override {};
		virtual void onSleep(PxActor** actors, PxU32 count) override {};
		virtual void onTrigger(PxTriggerPair* pairs, PxU32 count) override {}
		virtual void onContact(const PxContactPairHeader& PairHeader, const PxContactPair* Pairs, PxU32 NumPairs) override;
		virtual void onAdvance(const PxRigidBody*const* bodyBuffer, const PxTransform* poseBuffer, const PxU32 count) override {}

		void AddCollisionNotifyInfo(const BodyInstance* Body0, const BodyInstance* Body1,
			const physx::PxContactPair * Pairs, uint32 NumPairs, std::vector<CollisionNotifyInfo>& PendingNotifyInfos);

	private:	
		PhysicScene* m_OwningScene;
	};

	class PhysicScene
	{
	public:

		PhysicScene(World* InWorld);
		~PhysicScene();

		void Release();

		inline World* GetOwningWorld() { return m_OwningWorld; }
		inline physx::PxScene* GetPhysxScene() { return m_PhysxScene; }

		virtual void Tick(float DeltaTime);

		inline bool IsSimulating() const;

		void AddActor(physx::PxActor* InActor);
		void RemoveActor(physx::PxActor* InActor);

		void DrawDebugCollisions();
		void DrawDebugForRigidActor(PxRigidActor* RigidActor);

		//TODO: add force, clear velocity, set target, add actor, ...

		inline void ToggleShowCollision() { m_DrawDebugCollision = !m_DrawDebugCollision; }

	private:

		void StepSimulation(float DeltaTime);
		void SyncActors();

		void DispatchPhysicEvents();

		static PxFilterFlags contactReportFilterShader(PxFilterObjectAttributes attributes0, PxFilterData filterData0, 
												PxFilterObjectAttributes attributes1, PxFilterData filterData1,
												PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize);

		World* m_OwningWorld;
		physx::PxScene* m_PhysxScene;

		physx::PxDefaultCpuDispatcher* m_Dispatcher;
		physx::PxSimulationEventCallback* m_SimEventCallback;

		physx::PxMaterial* m_Material;

		std::vector<CollisionNotifyInfo> m_PendingCollisionNotifies;

		bool m_DrawDebugCollision = true;

		friend class LevelViewportGuiLayer;
		friend class AssetPreviewStaticMeshGuiLayer;
		friend class PhysXSimEventCallback;
	};
}