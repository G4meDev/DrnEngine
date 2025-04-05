#pragma once

#include "ForwardTypes.h"
#include "Actor.h"

namespace Drn
{
	class StaticMeshActor : public Actor
	{
	public:
		StaticMeshActor();
		virtual ~StaticMeshActor();

		virtual void Tick(float DeltaTime) override;

		inline StaticMeshComponent* GetMeshComponent() { return m_MeshComponenet.get(); }

		inline virtual EActorType GetActorType() override { return EActorType::StaticMeshActor; }

		virtual void Serialize(Archive& Ar) override;

	protected:

		std::unique_ptr<StaticMeshComponent> m_MeshComponenet;

	private:

	};
}