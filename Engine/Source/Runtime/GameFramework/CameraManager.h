#pragma once

#include "ForwardTypes.h"
#include "Runtime/Engine/Actor.h"

namespace Drn
{
	class CameraManager : public Actor
	{
	public:
		CameraManager();
		virtual ~CameraManager();

		void Serialize( Archive& Ar ) override;

		void Tick( float DeltaTime ) override;

		EActorType GetActorType() override { return EActorType::CameraManager; };
		inline static EActorType GetActorTypeStatic() { return EActorType::CameraManager; };

		void SetViewTarget(Actor* Target);
		void OnViewTargetDestroyed(Actor* Target);

		inline ViewInfo GetViewInfo() const { return m_ViewInfo; }

 protected:

		ViewInfo m_ViewInfo;
		Actor* m_ViewTarget;

		std::shared_ptr<SceneComponent> m_RootComponent;
	};
}