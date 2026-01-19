#pragma once

#include "ForwardTypes.h"
#include "Runtime/Engine/ReflectionCaptureComponent.h"

namespace Drn
{
	class SphereReflectionCaptureComponent : public ReflectionCaptureComponent
	{
	public:
		SphereReflectionCaptureComponent();
		virtual ~SphereReflectionCaptureComponent();

		virtual void Serialize( Archive& Ar ) override;

		inline virtual float GetInfluenceBoundingRadius() const override { return InfluenceRadius; }

#if WITH_EDITOR
		virtual void DrawDetailPanel(float DeltaTime) override;

		void DrawAttenuation();

		void DrawEditorDefault() override;
		void DrawEditorSelected() override;
#endif

	protected:
		float InfluenceRadius;

		virtual void RegisterComponent(World* InOwningWorld) override;
		virtual void UnRegisterComponent() override;

		virtual void OnUpdateTransform( bool SkipPhysic ) override;

#if WITH_EDITOR
		inline virtual bool HasSprite() const override { return true; }
#endif
	};
}