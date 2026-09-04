#pragma once

#include "ForwardTypes.h"
#include "Actor.h"

namespace Drn
{
	class SkeletalMeshActor : public Actor
	{
		public:
			SkeletalMeshActor();
			virtual ~SkeletalMeshActor();

			virtual void Tick(float DeltaTime) override;

			inline SkeletalMeshComponent* GetMeshComponent() { return m_MeshComponenet.get(); }

			inline virtual EActorType GetActorType() override { return EActorType::SkeletalMeshActor; }

			virtual void Serialize(Archive& Ar) override;

		protected:

			std::unique_ptr<SkeletalMeshComponent> m_MeshComponenet;

		private:

	};
}