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

		void Possess(class Pawn* InPawn);
		void UnPossess();

		virtual void OnPossess(class Pawn* InPawn);
		virtual void OnUnPossess();

		inline class Pawn* GetPawn() const { return m_Pawn; }

#if WITH_EDITOR
		virtual bool DrawDetailPanel() override;
#endif

	protected:

		Pawn* m_Pawn;

		std::shared_ptr<SceneComponent> m_RootComponent;
	};
}