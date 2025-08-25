#pragma once

#include "ForwardTypes.h"
#include "Runtime/Engine/Actor.h"

namespace Drn
{
	class Controller : public Actor
	{
	public:

		Controller();
		virtual ~Controller();

		void Tick( float DeltaTime ) override;
		EActorType GetActorType() override { return EActorType::Controller; };
		inline static EActorType GetActorTypeStatic() { return EActorType::Controller; };
		void Serialize( Archive& Ar ) override;

#if WITH_EDITOR
		virtual bool DrawDetailPanel() override;
#endif

	protected:

		std::shared_ptr<SceneComponent> m_RootComponent;
	};
}