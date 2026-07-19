#pragma once

#include "ForwardTypes.h"

#include <PxPhysics.h>
#include <PxPhysicsAPI.h>

using namespace physx;

namespace Drn
{
	enum class EHitFlags : uint16
	{
		None = 0,
		Position = (1 << 0),
		Normal = (1 << 1),
		UV = (1 << 3),
		MTD = (1 << 9),
		FaceIndex = (1 << 10)
	};

	enum class EQueryFlags : uint16
	{
		None = 0,
		PreFilter = (1 << 2),
		PostFilter = (1 << 3),
		AnyHit = (1 << 4),
		SkipNarrowPhase = (1 << 5)
	};

	Vector P2Vector(const PxVec3& Vec);
	Vector Pd2Vector(const PxVec3d& Vec);
	PxVec3 Vector2P(const Vector& Vec);
	PxVec3d Vector2Pd(const Vector& Vec);

	Quat P2Quat(const PxQuat& Q);
	PxQuat Quat2P(const Quat& Q);

	Transform P2Transform(const PxTransform& T);
	PxTransform Transform2P(const Transform& T);

	inline PxHitFlags U2PHitFlags(const EHitFlags& Flags)
	{
		uint32 Result = 0;
		if (EnumHasAnyFlags(Flags, EHitFlags::Position))
		{
			Result |= PxHitFlag::ePOSITION;
		}

		if (EnumHasAnyFlags(Flags, EHitFlags::Normal))
		{
			Result |= PxHitFlag::eNORMAL;
		}

		if (EnumHasAnyFlags(Flags, EHitFlags::UV))
		{
			Result |= PxHitFlag::eUV;
		}

		if (EnumHasAnyFlags(Flags, EHitFlags::MTD))
		{
			Result |= PxHitFlag::eMTD;
		}

		if (EnumHasAnyFlags(Flags, EHitFlags::FaceIndex))
		{
			Result |= PxHitFlag::eFACE_INDEX;
		}

		return (PxHitFlags)Result;
	}

	inline EHitFlags P2UHitFlags(const PxHitFlags& Flags)
	{
		EHitFlags Result = EHitFlags::None;
		if (Flags & PxHitFlag::ePOSITION)
		{
			EnumAddFlags(Result, EHitFlags::Position);
		}

		if (Flags & PxHitFlag::eNORMAL)
		{
			EnumAddFlags(Result, EHitFlags::Normal);
		}

		if (Flags & PxHitFlag::eUV)
		{
			EnumAddFlags(Result, EHitFlags::UV);
		}

		if (Flags & PxHitFlag::eMTD)
		{
			EnumAddFlags(Result, EHitFlags::MTD);
		}

		if (Flags & PxHitFlag::eFACE_INDEX)
		{
			EnumAddFlags(Result, EHitFlags::FaceIndex);
		}

		return Result;
	}

	inline EQueryFlags P2UQueryFlags(PxQueryFlags Flags)
	{
		EQueryFlags Result = EQueryFlags::None;
		if (Flags & PxQueryFlag::ePREFILTER)
		{
			EnumAddFlags(Result, EQueryFlags::PreFilter);
		}

		if (Flags & PxQueryFlag::ePOSTFILTER)
		{
			EnumAddFlags(Result, EQueryFlags::PostFilter);
		}

		if (Flags & PxQueryFlag::eANY_HIT)
		{
			EnumAddFlags(Result, EQueryFlags::AnyHit);
		}

		return Result;
	}

	inline PxQueryFlags U2PQueryFlags(EQueryFlags Flags)
	{
		uint32 Result = 0;
		if (EnumHasAnyFlags(Flags, EQueryFlags::PreFilter))
		{
			Result |= PxQueryFlag::ePREFILTER;
		}

		if (EnumHasAnyFlags(Flags, EQueryFlags::PostFilter))
		{
			Result |= PxQueryFlag::ePOSTFILTER;
		}

		if (EnumHasAnyFlags(Flags, EQueryFlags::AnyHit))
		{
			Result |= PxQueryFlag::eANY_HIT;
		}

		return (PxQueryFlags)Result;
	}

	struct CollisionObjectQueryParams
	{
		int32 ObjectTypesToQuery;
		uint8 IgnoreMask;

		CollisionObjectQueryParams()
			: ObjectTypesToQuery(0)
			, IgnoreMask(0)
		{
		}

		CollisionObjectQueryParams(ECollisionChannel QueryChannel)
		{
			ObjectTypesToQuery = ECC_TO_BITFIELD(QueryChannel);
			IgnoreMask = 0;
		}

		CollisionObjectQueryParams(const std::vector<ECollisionChannel>& ObjectTypes)
		{
			ObjectTypesToQuery = 0;

			for ( const ECollisionChannel& Iter : ObjectTypes )
			{
				AddObjectTypesToQuery(Iter);
			}

			IgnoreMask = 0;
		}

		CollisionObjectQueryParams(int32 InObjectTypesToQuery)
		{
			ObjectTypesToQuery = InObjectTypesToQuery;
			IgnoreMask = 0;
		}

		void AddObjectTypesToQuery(ECollisionChannel QueryChannel)
		{
			ObjectTypesToQuery |= ECC_TO_BITFIELD(QueryChannel);
		}

		void RemoveObjectTypesToQuery(ECollisionChannel QueryChannel)
		{
			ObjectTypesToQuery &= ~ECC_TO_BITFIELD(QueryChannel);
		}

		int32 GetQueryBitfield() const
		{
			drn_check(IsValid());

			return ObjectTypesToQuery;
		}

		bool IsValid() const
		{ 
			return (ObjectTypesToQuery != 0); 
		}
	};

	CollisionFilterData CreateObjectQueryFilterData(const int32 MultiTrace, const CollisionObjectQueryParams& ObjectParam);

	PxFilterData U2PFilterData(const CollisionFilterData& FilterData);
	CollisionFilterData P2UFilterData(const PxFilterData& PFilterData);

// -----------------------------------------------------------------------------------------------------------------------------------------

	enum class EQueryMobilityType
	{
		Any,
		Static,
		Dynamic
	};

	struct CollisionQueryParams
	{
		std::string OwnerTag;

		bool bFindInitialOverlaps;
		bool bReturnFaceIndex;
		bool bReturnPhysicalMaterial;
		bool bIgnoreBlocks;
		bool bIgnoreTouches;
		bool bSkipNarrowPhase;
		EQueryMobilityType MobilityType;

		typedef std::vector<uint32> IgnoreComponentsArrayType;
		typedef std::vector<uint32> IgnoreActorsArrayType;
		uint8 IgnoreMask;

	private:

		mutable IgnoreComponentsArrayType IgnoreComponents;
		IgnoreActorsArrayType IgnoreActors;

		void Internal_AddIgnoredComponent(const PrimitiveComponent* InIgnoreComponent);

	public:

		const IgnoreComponentsArrayType& GetIgnoredComponents() const;

		const IgnoreActorsArrayType& GetIgnoredActors() const
		{
			return IgnoreActors;
		}

		void ClearIgnoredComponents()
		{
			IgnoreComponents.clear();
		}

		void ClearIgnoredActors()
		{
			IgnoreActors.clear();
		}

		void SetNumIgnoredComponents(int32 NewNum);

		CollisionQueryParams(const Actor* InIgnoreActor = NULL);

		void AddIgnoredActor(const Actor* InIgnoreActor);
		void AddIgnoredActor(const uint32 InIgnoreActorID);

		void AddIgnoredActors(const std::vector<Actor*>& InIgnoreActors);
		void AddIgnoredActors(const std::vector<const Actor*>& InIgnoreActors);

		void AddIgnoredComponent(const PrimitiveComponent* InIgnoreComponent);
		void AddIgnoredComponents(const std::vector<PrimitiveComponent*>& InIgnoreComponents);

		static CollisionQueryParams DefaultQueryParam;
	};

	inline PxQueryFlags StaticDynamicQueryFlags(const CollisionQueryParams& Params)
	{
		switch (Params.MobilityType)
		{
		case EQueryMobilityType::Any: return  PxQueryFlag::eSTATIC | PxQueryFlag::eDYNAMIC;
		case EQueryMobilityType::Static: return  PxQueryFlag::eSTATIC;
		case EQueryMobilityType::Dynamic: return  PxQueryFlag::eDYNAMIC;
		default: drn_check(false);
		}

		drn_check(false);
		return PxQueryFlag::eSTATIC | PxQueryFlag::eDYNAMIC;
	}

	enum class ECollisionQueryHitType : uint8
	{
		None = 0,
		Touch = 1,
		Block = 2
	};

	inline ECollisionChannel GetCollisionChannelAndExtraFilter(uint32 Word3, uint8& OutMaskFilter)
	{
		uint32 ChannelMask = GetCollisionChannel(Word3);
		OutMaskFilter = Word3 >> (32 - 6);
		return (ECollisionChannel)ChannelMask;
	}

	ECollisionQueryHitType CalcQueryHitType(const CollisionFilterData& QueryFilter, const CollisionFilterData& ShapeFilter, bool bPreFilter = false);

	void SetHitResultFromShapeAndFaceIndex(const PxShape& Shape,  const PxActor& Actor, const uint32 FaceIndex, const Vector& HitLocation, HitResult& OutResult, bool bReturnPhysMat);
}