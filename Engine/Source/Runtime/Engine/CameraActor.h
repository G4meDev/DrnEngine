#pragma once

#include "ForwardTypes.h"
#include "Actor.h"

#include "Editor/Misc/ViewportCameraInputHandler.h"

namespace Drn
{
	class CameraActor : public Actor
	{
	public:
		CameraActor();
		virtual ~CameraActor();

		virtual void Tick(float DeltaTime) override;

		inline CameraComponent* GetCameraComponent() { return m_CameraComponenet.get(); }

#if WITH_EDITOR
		void ApplyViewportInput( const ViewportCameraInputHandler& CameraInput, float CameraMovementSpeed,
			float CameraRotationSpeed );
#endif

	protected:

		std::unique_ptr<CameraComponent> m_CameraComponenet;

	private:

	};
}