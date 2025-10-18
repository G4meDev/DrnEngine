#include "DrnPCH.h"
#include "EngineTypes.h"

#include "Editor/Misc/EditorMisc.h"

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

	EngineTypes* EngineTypes::m_SingletonInstance;

	void EngineTypes::RegisterSerializableActor( EActorType ActorType, std::function<Actor*( World* InWorld, Archive& Ar )>&& Func )
	{
		EngineTypes::Get()->m_ActorSerializationMap[ActorType] = Func;
	}

	void EngineTypes::Register()
	{
		REGISTER_SERIALIZABLE_ACTOR( EActorType::StaticMeshActor		, StaticMeshActor );
		REGISTER_SERIALIZABLE_ACTOR( EActorType::PointLight				, PointLightActor );
		REGISTER_SERIALIZABLE_ACTOR( EActorType::SpotLight				, SpotLightActor );
		REGISTER_SERIALIZABLE_ACTOR( EActorType::DirectionalLight		, DirectionalLightActor );
		REGISTER_SERIALIZABLE_ACTOR( EActorType::SkyLight				, SkyLightActor );
		REGISTER_SERIALIZABLE_ACTOR( EActorType::PostProcessVolume		, PostProcessVolume );
		REGISTER_SERIALIZABLE_ACTOR( EActorType::DecalActor				, DecalActor );
		REGISTER_SERIALIZABLE_ACTOR( EActorType::Pawn					, Pawn );
		REGISTER_SERIALIZABLE_ACTOR( EActorType::Character				, Character );

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