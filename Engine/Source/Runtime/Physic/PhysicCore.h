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
}