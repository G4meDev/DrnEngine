#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	struct RigidBodyContactInfo
	{
		RigidBodyContactInfo()
			: ContactPosition(Vector::ZeroVector)
			, ContactNormal(Vector::ZeroVector)
		{
		}

		RigidBodyContactInfo(const Vector& InContactPosition, const Vector& InContactNormal) 
			: ContactPosition(InContactPosition)
			, ContactNormal(InContactNormal)
		{
		}

		Vector ContactPosition;
		Vector ContactNormal;

		void SwapOrder();
	};

	struct CollisionImpactData
	{
	public:
		CollisionImpactData()
		{}

		std::vector<RigidBodyContactInfo> ContactInfos;
		void SwapContactOrders();
	};

	struct RigidBodyCollisionInfo
	{
		RigidBodyCollisionInfo()
			: m_Actor(nullptr)
			, m_Component(nullptr)
		{}
		
		Actor* m_Actor;
		PrimitiveComponent* m_Component;

		void SetFrom(const BodyInstance* BodyInst);
		BodyInstance* GetBodyInstance() const;
	};

	struct CollisionNotifyInfo
	{
		CollisionNotifyInfo() :
			bCallEvent0(true),
			bCallEvent1(true)
		{}

		bool bCallEvent0;
		bool bCallEvent1;

		RigidBodyCollisionInfo Info0;
		RigidBodyCollisionInfo Info1;

		CollisionImpactData RigidCollisionData;

		bool IsValidForNotify() const;
	};
}