#pragma once

#include "ForwardTypes.h"
#include "Actor.h"

namespace Drn
{
	class InstancedStaticMeshActor : public Actor
	{
	public:
		InstancedStaticMeshActor();
		virtual ~InstancedStaticMeshActor();

		virtual void Tick(float DeltaTime) override;

		inline InstancedStaticMeshComponent* GetInstancedStaticMeshComponent() { return m_InstancedStaticMeshComponenet.get(); }

		inline virtual EActorType GetActorType() override { return EActorType::InstancedStaticMeshActor; }

		//virtual void Serialize(Archive& Ar) override;

	protected:

		std::unique_ptr<InstancedStaticMeshComponent> m_InstancedStaticMeshComponenet;

	private:

	};
}