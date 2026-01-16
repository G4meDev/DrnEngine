#pragma once

#include "ForwardTypes.h"
#include "Runtime/Engine/ReflectionCapture.h"

namespace Drn
{
	class SphereReflectionCaptureComponent;

	class SphereReflectionCapture : public ReflectionCapture
	{
	public:
		SphereReflectionCapture();
		virtual ~SphereReflectionCapture();

		inline SphereReflectionCaptureComponent* GetSphereReflectionCapture() const { return m_SphereReflectionCaptureComponent.get(); }

		virtual EActorType GetActorType() override { return EActorType::SphereReflectionCapture; }
		static EActorType GetActorTypeStatic() { return EActorType::SphereReflectionCapture; }

	protected:
		std::unique_ptr<SphereReflectionCaptureComponent> m_SphereReflectionCaptureComponent;
	};
}