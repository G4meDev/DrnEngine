#pragma once

#include "ForwardTypes.h"
#include "Runtime/Engine/Actor.h"
#include "Runtime/Core/Archive.h"

namespace Drn
{
	class DecalActor : public Actor
	{
	public:
		DecalActor();
		virtual ~DecalActor();

		virtual void Serialize( Archive& Ar ) override;

		virtual void Tick( float DeltaTime ) override {};
		inline virtual EActorType GetActorType() override { return EActorType::DecalActor; }

		std::unique_ptr<class DecalComponent> m_DecalComponent;
	};
}