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

		virtual void SetupPlayerInputComponent( class InputComponent* PlayerInputComponent );

		virtual CameraComponent* GetViewCamera() const { return nullptr; }

#if WITH_EDITOR
		virtual bool DrawDetailPanel() override;
#endif

	protected:

		void CreatePlayerInputComponent();
		void DestroyPlayerInputComponent();

		void OnMoveForward( float Value );
		void OnMoveRight(float Value);

		void OnLookUp(float Value);
		void OnLookRight(float Value);

		std::shared_ptr<SceneComponent> m_RootComponent;
		class InputComponent* m_InputComponent;

		Controller* m_Controller;

		bool m_AutoPossessPlayer;
	};
}