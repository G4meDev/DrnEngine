#pragma once

#include "ForwardTypes.h"
#include "LightComponent.h"

namespace Drn
{
	class PointLightComponent : public LightComponent
	{
	public:
		PointLightComponent();
		virtual ~PointLightComponent();

		virtual void Serialize( Archive& Ar ) override;

		inline float GetRadius() const { return m_Radius; }
		inline float GetMaxDrawDistance() const { return MaxDrawDistance; };
		Matrix GetLocalToWorld() const;

		void SetRadius( float Radius );
		void SetMaxDrawDistance( float InMaxDrawDistance );

#if WITH_EDITOR
		virtual void DrawDetailPanel(float DeltaTime) override;

		void DrawAttenuation();

		void DrawEditorDefault() override;
		void DrawEditorSelected() override;
#endif

	protected:

		virtual void RegisterComponent(World* InOwningWorld) override;
		virtual void UnRegisterComponent() override;

		virtual void OnUpdateTransform( bool SkipPhysic ) override;

		float m_Radius;
		float MaxDrawDistance;

		class PointLightSceneProxy* m_PointLightSceneProxy;

#if WITH_EDITOR
		inline virtual bool HasSprite() const override { return true; }

#endif

	private:
	};
}