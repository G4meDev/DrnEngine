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

		std::vector<int32> AddCollisionNotifyInfo( const BodyInstance* Body0, const BodyInstance* Body1,
			const physx::PxContactPair * Pairs, uint32 NumPairs, std::vector<CollisionNotifyInfo>& PendingNotifyInfos);

	private:	
		PhysicScene* m_OwningScene;
	};

	class CollisionQueryFilterCallback : public PxQueryFilterCallback
	{
	public:
		ECollisionQueryHitType PreFilterReturnValue;

		const std::vector<uint32>& IgnoreComponents;
		const std::vector<uint32>& IgnoreActors;

		bool bIsOverlapQuery;
		bool bIgnoreTouches;
		bool bIgnoreBlocks;

		bool bDiscardInitialOverlaps;
		bool bIsSweep;

		CollisionQueryFilterCallback(const CollisionQueryParams& InQueryParams, bool bInIsSweep)
			: IgnoreComponents(InQueryParams.GetIgnoredComponents())
			, IgnoreActors(InQueryParams.GetIgnoredActors())
			, bIsSweep(bInIsSweep)
		{
			PreFilterReturnValue = ECollisionQueryHitType::None;
			bIsOverlapQuery = false;
			bIgnoreTouches = InQueryParams.bIgnoreTouches;
			bIgnoreBlocks = InQueryParams.bIgnoreBlocks;
			bDiscardInitialOverlaps = !InQueryParams.bFindInitialOverlaps;
		}

		virtual PxQueryHitType::Enum preFilter(const PxFilterData& filterData, const PxShape* shape, const PxRigidActor* actor, PxHitFlags& queryFlags) override;
		virtual PxQueryHitType::Enum postFilter(const PxFilterData& filterData, const PxQueryHit& hit, const PxShape* shape, const PxRigidActor* actor) override;
	};

	class PhysicScene
	{
	public:

		PhysicScene(World* InWorld);
		~PhysicScene();

		void Release();

		inline World* GetOwningWorld() { return m_OwningWorld; }
		inline physx::PxScene* GetPhysxScene() { return m_PhysxScene; }
		inline physx::PxControllerManager* GetControllerManager() { return m_ControllerManager; }

		virtual void Tick(float DeltaTime);

		inline bool IsSimulating() const;

		void AddActor(physx::PxActor* InActor);
		void RemoveActor(physx::PxActor* InActor);

		void DrawDebugCollisions();
		void DrawDebugForRigidActor(PxRigidActor* RigidActor);

		bool RaycastSingle(const World* InWorld, HitResult& OutHit, const Vector Start, const Vector End, const CollisionQueryParams& Params
			, const CollisionObjectQueryParams& ObjectParams);

		bool RaycastMulti(const World* InWorld, std::vector<HitResult>& OutHits, const Vector Start, const Vector End, const CollisionQueryParams& Params
			, const CollisionObjectQueryParams& ObjectParams);

		bool RaycastTest(const World* InWorld, const Vector Start, const Vector End, const CollisionQueryParams& Params
			, const CollisionObjectQueryParams& ObjectParams);

		bool GeomSweepSingle(const World* InWorld, HitResult& OutHit, const PxGeometry& GeomInputs, const Vector Start, const Vector End, const Quat& Rotation, const CollisionQueryParams& Params
			, const CollisionObjectQueryParams& ObjectParams);

		//bool GeomSweepMulti(const World* InWorld, std::vector<HitResult>& OutHits, const Vector Start, const Vector End, const CollisionQueryParams& Params
		//	, const CollisionObjectQueryParams& ObjectParams);
		//
		//bool GeomSweepTest(const World* InWorld, const Vector Start, const Vector End, const CollisionQueryParams& Params
		//	, const CollisionObjectQueryParams& ObjectParams);

		template <typename BufferType, typename ElementType>
		void ConvertTraceResults(bool& OutHasValidBlockingHit, const World* InWorld, int32 NumHits, BufferType* Hits, float CheckLength, const CollisionFilterData& QueryFilter, HitResult& OutHits, const Vector& StartLoc, const Vector& EndLoc,
			const PxGeometry* Geom, const Transform& QueryTM, float MaxDistance, bool bReturnFaceIndex, bool bReturnPhysMat);

		template <typename BufferType, typename ElementType>
		void ConvertTraceResults(bool& OutHasValidBlockingHit, const World* InWorld, int32 NumHits, BufferType* Hits, float CheckLength, const CollisionFilterData& QueryFilter, std::vector<HitResult>& OutHits, const Vector& StartLoc, const Vector& EndLoc,
			const PxGeometry* Geom, const Transform& QueryTM, float MaxDistance, bool bReturnFaceIndex, bool bReturnPhysMat);

		void ConvertQueryImpactHit(const World* InWorld, const PxLocationHit& PHit, const PxActorShape& PActorShape, HitResult& OutResult, float CheckLength, const CollisionFilterData& QueryFilter, const Vector& StartLoc, const Vector& EndLoc, const PxGeometry* Geom, const Transform& QueryTM, bool bReturnFaceIndex, bool bReturnPhysMat);

		bool ConvertOverlappedShapeToImpactHit(const World* InWorld, const PxLocationHit& PHit, const PxActorShape& PActorShape, const Vector& StartLoc, const Vector& EndLoc,
			HitResult& OutResult, const Transform& QueryTM, const CollisionFilterData& QueryFilter, bool bReturnPhysMat);

	private:

		void StepSimulation(float DeltaTime);
		void SyncActors();

		void DispatchPhysicEvents();

		static PxFilterFlags PhysXSimFilterShader(PxFilterObjectAttributes attributes0, PxFilterData filterData0, 
												PxFilterObjectAttributes attributes1, PxFilterData filterData1,
												PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize);

		World* m_OwningWorld;
		physx::PxScene* m_PhysxScene;
		physx::PxControllerManager* m_ControllerManager;

		physx::PxDefaultCpuDispatcher* m_Dispatcher;
		physx::PxSimulationEventCallback* m_SimEventCallback;

		physx::PxMaterial* m_Material;

		std::vector<CollisionNotifyInfo> m_PendingCollisionNotifies;

		friend class LevelViewportGuiLayer;
		friend class AssetPreviewStaticMeshGuiLayer;
		friend class PhysXSimEventCallback;
	};
}