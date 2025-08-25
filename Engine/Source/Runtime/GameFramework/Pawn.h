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

		inline bool GetAutoPossessPlayer() const { return m_AutoPossessPlayer; }

		virtual void PossessBy(class Controller* InController);
		virtual void UnPossess();

#if WITH_EDITOR
		virtual bool DrawDetailPanel() override;
#endif

	protected:

		void CreatePlayerInputComponent();
		void DestroyPlayerInputComponent();

		std::shared_ptr<SceneComponent> m_RootComponent;
		std::shared_ptr<StaticMeshComponent> m_MeshComponent;
		class InputComponent* m_InputComponent;

		Controller* m_Controller;

		bool m_AutoPossessPlayer;
	};
}