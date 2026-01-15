#pragma once

#include "ForwardTypes.h"
#include "Runtime/Engine/Actor.h"

namespace Drn
{
	class ReflectionCaptureComponent;

	class ReflectionCapture : public Actor
	{
	public:
		ReflectionCaptureComponent* GetCaptureComponent() const { return m_CaptureComponent; }

	protected:
		ReflectionCapture() : Actor(), m_CaptureComponent(nullptr) {};
		virtual ~ReflectionCapture() {};

		ReflectionCaptureComponent* m_CaptureComponent;
	};
}