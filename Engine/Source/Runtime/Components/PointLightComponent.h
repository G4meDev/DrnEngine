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
		inline float GetDepthBias() const { return m_DepthBias; }
		Matrix GetLocalToWorld() const;

		void SetRadius( float Radius );
		void SetDepthBias( float Bias );

	protected:

		virtual void RegisterComponent(World* InOwningWorld) override;
		virtual void UnRegisterComponent() override;

		virtual void OnUpdateTransform( bool SkipPhysic ) override;

		float m_Radius;
		float m_DepthBias;
		class PointLightSceneProxy* m_PointLightSceneProxy;

#if WITH_EDITOR
		inline virtual bool HasSprite() const override { return true; }

		virtual void DrawDetailPanel(float DeltaTime) override;
#endif

	private:
	};
}