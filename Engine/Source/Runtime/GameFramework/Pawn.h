#pragma once

#include "ForwardTypes.h"
#include "Runtime/Engine/Actor.h"

namespace Drn
{
	class Pawn : public Actor
	{
	public:

		Pawn();
		virtual ~Pawn();

		void Tick( float DeltaTime ) override;
		EActorType GetActorType() override;
		inline static EActorType GetActorTypeStatic() { return EActorType::Pawn; };
		void Serialize( Archive& Ar ) override;

	protected:
		std::shared_ptr<SceneComponent> m_RootComponent;
		std::shared_ptr<StaticMeshComponent> m_MeshComponent;

		bool m_AutoPossessPlayer;
	};
}