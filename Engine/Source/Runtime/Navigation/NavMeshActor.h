#pragma once

#include "ForwardTypes.h"
#include "Runtime/Engine/DynamicMeshComponent.h"

namespace Drn
{
	class NavMeshComponent;

	class NavMeshActor : public Actor
	{
	public:
		NavMeshActor();
		virtual ~NavMeshActor();

		virtual void Serialize(Archive& Ar) override;

		virtual void Tick(float DeltaTime) override {};
		inline virtual EActorType GetActorType() override { return EActorType::NavMeshActor; }

		std::unique_ptr<NavMeshComponent> m_NavMeshComponent;
		std::unique_ptr<DynamicMeshComponent> m_DynamicMeshComponent;
	};
}