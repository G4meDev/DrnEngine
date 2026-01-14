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

		inline Vector GetDirection() const { return GetWorldRotation().GetVector(); }
		inline float GetAttenuation() const { return m_Attenuation; }
		inline float GetOutterRadius() const { return m_OuterRadius; }
		inline float GetInnerRadius() const { return m_InnerRadius; }

		inline float GetMaxDrawDistance() const { return MaxDrawDistance; }

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

		void SetMaxDrawDistance(float InMaxDrawDistance);
		virtual void SetCastStaticShadow( bool bInCastShadow ) override;

		inline bool CanUseStaticShadowmap() const { return IsStatic() && bCastStaticShadow; }
		inline TRefCountPtr<RenderTexture2D>& GetCachedShadowmap() { return CachedShadowmap; }
		inline StaticShadowDepthMapData& GetCachedShadowmapData() { return CachedShadowmapData; }

#if WITH_EDITOR
		bool IsRequiredShadowBake() const { return bRequiredStaticShadowmapBake; }
		void ClearRequiredShadowBake() { bRequiredStaticShadowmapBake = false; }
#endif

	protected:
		virtual void RegisterComponent( World* InOwningWorld ) override;
		virtual void UnRegisterComponent() override;
	
		virtual void OnUpdateTransform( bool SkipPhysic ) override;
		virtual void SetStatic(bool bInStatic) override;

		void InvalidateCachedShadow() { CachedShadowmap = nullptr; CachedShadowmapData.Empty(); }

		float m_Attenuation;
		float m_OuterRadius;
		float m_InnerRadius;

		float MaxDrawDistance;

		TRefCountPtr<class RenderTexture2D> CachedShadowmap;
		StaticShadowDepthMapData CachedShadowmapData;

		class SpotLightSceneProxy* m_SpotLightSceneProxy;

#if WITH_EDITOR
		inline virtual bool HasSprite() const override
		{
			return true;
		}
	
		virtual void DrawDetailPanel( float DeltaTime ) override;

		void DrawAttenuation();

		virtual void DrawEditorDefault() override;
		virtual void DrawEditorSelected() override;

		bool bRequiredStaticShadowmapBake = false;
#endif
	
	private:
	};
}