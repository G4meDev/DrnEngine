#include "DrnPCH.h"
#include "EngineTypes.h"

#include "Editor/Misc/EditorMisc.h"

namespace Drn
{
	//REGISTER_SERIALIZABLE_ACTOR( EActorType::SpotLight, SpotLightActor );
	//DECLARE_LEVEL_SPAWNABLE_CLASS( SpotLightActor, Light );

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

	EngineTypes* EngineTypes::m_SingletonInstance;
	SerializableActor::SerializableActor( EActorType InActorType, std::function<Actor*( World*, Archive& Ar )> InFunc )
	{
		EngineTypes::Get()->m_ActorSerializationMap[InActorType] = InFunc;
	}
	
	EngineTypes* EngineTypes::Get()
	{
		if ( !m_SingletonInstance )
		{
			m_SingletonInstance = new EngineTypes();
		}
		return m_SingletonInstance;
	}

}