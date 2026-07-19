#include "DrnPCH.h"
#include "PhysicCore.h"

namespace Drn
{
	Vector P2Vector( const PxVec3& Vec )
	{
		return Vector(Vec.x, Vec.y, Vec.z);
	}

	Vector Pd2Vector( const PxVec3d& Vec )
	{
		return Vector(Vec.x, Vec.y, Vec.z);
	}

	PxVec3 Vector2P( const Vector& Vec )
	{
		return PxVec3(Vec.GetX(), Vec.GetY(), Vec.GetZ());
	}

	PxVec3d Vector2Pd( const Vector& Vec )
	{
		return PxVec3d(Vec.GetX(), Vec.GetY(), Vec.GetZ());
	}

	Quat P2Quat( const PxQuat& Q )
	{
		return Quat(Q.x, Q.y, Q.z, Q.w);
	}

	PxQuat Quat2P( const Quat& Q )
	{
		return PxQuat(Q.GetX(), Q.GetY(), Q.GetZ(), Q.GetW());
	}

	Transform P2Transform( const PxTransform& T )
	{
		return Transform( P2Vector(T.p), P2Quat(T.q), Vector::OneVector);
	}

	PxTransform Transform2P( const Transform& T )
	{
		return PxTransform(Vector2P(T.GetLocation()), Quat2P(T.GetRotation()));
	}

	CollisionFilterData CreateObjectQueryFilterData( const int32 MultiTrace, const CollisionObjectQueryParams& ObjectParam )
	{
		CollisionFilterData NewData;
		//NewData.Word0 = (uint32)ECollisionQuery::ObjectQuery;

		//if (bTraceComplex)
		//{
		//	NewData.Word3 |= EPDF_ComplexCollision;
		//}
		//else
		//{
		//	NewData.Word3 |= EPDF_SimpleCollision;
		//}

		NewData.Word1 = ObjectParam.GetQueryBitfield();
		NewData.Word3 |= CreateChannelAndFilter((ECollisionChannel)MultiTrace, ObjectParam.IgnoreMask);
	
		return NewData;
	}

	PxFilterData U2PFilterData( const CollisionFilterData& FilterData )
	{
		return PxFilterData(FilterData.Word0, FilterData.Word1, FilterData.Word2, FilterData.Word3);
	}

	CollisionFilterData P2UFilterData( const PxFilterData& PFilterData )
	{
		CollisionFilterData FilterData;
		FilterData.Word0 = PFilterData.word0;
		FilterData.Word1 = PFilterData.word1;
		FilterData.Word2 = PFilterData.word2;
		FilterData.Word3 = PFilterData.word3;
		return FilterData;
	}

	ECollisionQueryHitType CalcQueryHitType( const CollisionFilterData& QueryFilter, const CollisionFilterData& ShapeFilter, bool bPreFilter )
	{
		uint8 QuerierMaskFilter;
		const ECollisionChannel QuerierChannel = GetCollisionChannelAndExtraFilter(QueryFilter.Word3, QuerierMaskFilter);

		uint8 ShapeMaskFilter;
		const ECollisionChannel ShapeChannel = GetCollisionChannelAndExtraFilter(ShapeFilter.Word3, ShapeMaskFilter);

		if ((QuerierMaskFilter & ShapeMaskFilter) != 0)	//If ignore mask hit something, ignore it
		{
			return ECollisionQueryHitType::None;
		}

		const uint32 ShapeBit = ECC_TO_BITFIELD(ShapeChannel);

		const int32 MultiTrace = (int32)QuerierChannel;
		// do I belong to one of objects of interest?
		if (ShapeBit & QueryFilter.Word1)
		{
			if (bPreFilter)	//In the case of an object query we actually want to return all object types (or first in single case). So in PreFilter we have to trick physx by not blocking in the multi case, and blocking in the single case.
			{

				return MultiTrace ? ECollisionQueryHitType::Touch: ECollisionQueryHitType::Block;
			}
			else
			{
				return ECollisionQueryHitType::Block;	//In the case where an object query is being resolved for the user we just return a block because object query doesn't have the concept of overlap at all and block seems more natural
			}
		}

		return ECollisionQueryHitType::None;
	}

	void SetHitResultFromShapeAndFaceIndex( const PxShape& Shape, const PxActor& Actor, const uint32 FaceIndex, const Vector& HitLocation, HitResult& OutResult, bool bReturnPhysMat )
	{
		PrimitiveComponent* OwningComponent = nullptr;
		if(const BodyInstance* BodyInst = PhysicUserData::Get<BodyInstance>(Actor.userData))
		{
			//BodyInst = FPhysicsInterface::ShapeToOriginalBodyInstance(BodyInst, &Shape);

			//Normal case where we hit a body
			//OutResult.Item = BodyInst->InstanceBodyIndex;
			//const UBodySetupCore* BodySetup = BodyInst->BodySetup.Get();	//this data should be immutable at runtime so ok to check from worker thread.
			//if (BodySetup)
			//{
			//	OutResult.BoneName = BodySetup->BoneName;
			//}
			//
			//OwningComponent = BodyInst->OwnerComponent.Get();
		}
		//else if(const FCustomPhysXPayload* CustomPayload = GetUserData<FCustomPhysXPayload>(Shape))	//todo(ocohen): wrap with PHYSX
		//{
		//	//Custom payload case
		//	OwningComponent = CustomPayload->GetOwningComponent().Get();
		//	if(OwningComponent && OwningComponent->bMultiBodyOverlap)
		//	{
		//		OutResult.Item = CustomPayload->GetItemIndex();
		//		OutResult.BoneName = CustomPayload->GetBoneName();
		//	}
		//	else
		//	{
		//		OutResult.Item = INDEX_NONE;
		//		OutResult.BoneName = NAME_None;
		//
		//	}
		//}

		OutResult.PhysMaterial = nullptr;

		if( OwningComponent )
		{
			OutResult.HitActor = OwningComponent->GetOwningActor();
			OutResult.HitComponent = OwningComponent;

			if (bReturnPhysMat)
			{
				//if (const PhysicsMaterial* PhysicsMaterial = GetMaterialFromInternalFaceIndex(Shape, Actor, FaceIndex))
				//{
				//	OutResult.PhysMaterial = GetUserData(*PhysicsMaterial);
				//}
			}
		}

		//OutResult.FaceIndex = INDEX_NONE;
	}

	void CollisionQueryParams::Internal_AddIgnoredComponent( const PrimitiveComponent* InIgnoreComponent )
	{
		if (InIgnoreComponent)
		{
			IgnoreComponents.push_back(InIgnoreComponent->GetUniqueID());
		}
	}

	const CollisionQueryParams::IgnoreComponentsArrayType& CollisionQueryParams::GetIgnoredComponents() const
	{
		return IgnoreComponents;
	}

	void CollisionQueryParams::SetNumIgnoredComponents( int32 NewNum )
	{
		if (NewNum > 0)
		{
			if (NewNum < IgnoreComponents.size())
			{
				IgnoreComponents.resize(NewNum);
			}
		}
		else
		{
			ClearIgnoredComponents();
		}
	}

	CollisionQueryParams::CollisionQueryParams( const Actor* InIgnoreActor )
	{
		MobilityType = EQueryMobilityType::Any;
		bFindInitialOverlaps = true;
		bReturnFaceIndex = false;
		bReturnPhysicalMaterial = false;
		IgnoreMask = 0;
		bIgnoreBlocks = false;
		bIgnoreTouches = false;
		bSkipNarrowPhase = false;

		AddIgnoredActor(InIgnoreActor);
	}

	void CollisionQueryParams::AddIgnoredActor( const Actor* InIgnoreActor )
	{
		if (InIgnoreActor)
		{
			IgnoreActors.push_back(InIgnoreActor->GetUniqueID());
		}
	}

	void CollisionQueryParams::AddIgnoredActor( const uint32 InIgnoreActorID )
	{
		IgnoreActors.push_back(InIgnoreActorID);
	}

	void CollisionQueryParams::AddIgnoredActors( const std::vector<Actor*>& InIgnoreActors )
	{
		for (Actor* A : InIgnoreActors)
		{
			AddIgnoredActor(A);
		}
	}

	void CollisionQueryParams::AddIgnoredActors( const std::vector<const Actor*>& InIgnoreActors )
	{
		for (const Actor* A : InIgnoreActors)
		{
			AddIgnoredActor(A);
		}
	}

	void CollisionQueryParams::AddIgnoredComponent( const PrimitiveComponent* InIgnoreComponent )
	{
		if (InIgnoreComponent)
		{
			IgnoreComponents.push_back(InIgnoreComponent->GetUniqueID());
		}
	}

	void CollisionQueryParams::AddIgnoredComponents( const std::vector<PrimitiveComponent*>& InIgnoreComponents )
	{
		for (PrimitiveComponent* Prim : InIgnoreComponents)
		{
			AddIgnoredComponent(Prim);
		}
	}

	CollisionQueryParams CollisionQueryParams::DefaultQueryParam;

}  // namespace Drn