#pragma once

#include "ForwardTypes.h"
#include "LightComponent.h"

#include "Runtime/Engine/SpotLightSceneProxy.h"

namespace Drn
{
	class SpotLightComponent : public LightComponent
	{
	public:
		SpotLightComponent();
		virtual ~SpotLightComponent();
	
		virtual void Serialize( Archive& Ar ) override;

		void SetDepthBias( float Bias );
		inline float GetDepthBias() const { return m_DepthBias; };
	
		inline float GetAttenuation() const { return m_Attenuation; }
		inline float GetOutterRadius() const { return m_OuterRadius; }
		inline float GetInnerRadius() const { return m_InnerRadius; }

		inline void SetAttenuation( float Attenuation )
		{
			m_Attenuation = Attenuation;
			MarkRenderStateDirty();
		}

		inline void SetOutterRadius( float OuterRadius )
		{
			m_OuterRadius = OuterRadius;
			MarkRenderStateDirty();
		}

		inline void SetInnerRadius( float InnerRadius )
		{
			m_InnerRadius = InnerRadius;
			MarkRenderStateDirty();
		}

	protected:
		virtual void RegisterComponent( World* InOwningWorld ) override;
		virtual void UnRegisterComponent() override;
	
		virtual void OnUpdateTransform( bool SkipPhysic ) override;

		float m_DepthBias;

		float m_Attenuation;
		float m_OuterRadius;
		float m_InnerRadius;

		class SpotLightSceneProxy* m_SpotLightSceneProxy;

#if WITH_EDITOR
		inline virtual bool HasSprite() const override
		{
			return true;
		}
	
		virtual void DrawDetailPanel( float DeltaTime ) override;
#endif
	
	private:
	};
}