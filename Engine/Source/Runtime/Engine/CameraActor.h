#pragma once

#include "ForwardTypes.h"
#include "Actor.h"

namespace Drn
{
	class CameraActor : public Actor
	{
	public:
		CameraActor();
		virtual ~CameraActor();

		virtual void Tick(float DeltaTime) override;

		inline CameraComponent* GetCameraComponent() { return m_CameraComponenet.get(); }

	protected:

		std::unique_ptr<CameraComponent> m_CameraComponenet;

	private:

	};
}