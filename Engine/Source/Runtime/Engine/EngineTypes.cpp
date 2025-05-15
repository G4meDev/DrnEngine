#include "DrnPCH.h"
#include "EngineTypes.h"

namespace Drn
{
	void RigidBodyCollisionInfo::SetFrom( const BodyInstance* BodyInst )
	{
		if (BodyInst)
		{
			m_Component = BodyInst->GetOwnerComponent();
			m_Actor = m_Component->GetOwningActor();
		}
		else
		{
			m_Component = nullptr;
			m_Actor = nullptr;
		}
	}

	BodyInstance* RigidBodyCollisionInfo::GetBodyInstance() const
	{
		BodyInstance* Result = nullptr;
		
		if (m_Component)
		{
			Result = &m_Component->GetBodyInstance();
		}

		return Result;
	}

	void CollisionImpactData::SwapContactOrders()
	{
		for ( RigidBodyContactInfo& Info : ContactInfos )
		{
			Info.SwapOrder();
		}
	}

	void RigidBodyContactInfo::SwapOrder()
	{
		ContactNormal = ContactNormal * -1;
	}

}