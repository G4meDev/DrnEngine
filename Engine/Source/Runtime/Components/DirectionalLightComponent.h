#pragma once

#include "ForwardTypes.h"
#include "Runtime/Components/LightComponent.h"

namespace Drn
{
	class DirectionalLightComponent : public LightComponent
	{
	public:
		DirectionalLightComponent();
		virtual ~DirectionalLightComponent();

		virtual void Serialize( Archive& Ar ) override;

		void SetDepthBias( float Bias );
		inline float GetDepthBias() const { return m_DepthBias; };

	protected:

		virtual void RegisterComponent(World* InOwningWorld) override;
		virtual void UnRegisterComponent() override;

		virtual void OnUpdateTransform( bool SkipPhysic ) override;

		float m_DepthBias;

		class DirectionalLightSceneProxy* m_DirectionalLightSceneProxy;

#if WITH_EDITOR
		inline virtual bool HasSprite() const override { return true; }
		virtual void DrawDetailPanel( float DeltaTime ) override;
#endif
	};
}