#pragma once

#include "ForwardTypes.h"
#include "LightComponent.h"

namespace Drn
{
	class D3D12CommandList;
	class RenderTextureCube;

	class PointLightComponent : public LightComponent
	{
	public:
		PointLightComponent();
		virtual ~PointLightComponent();

		virtual void Serialize( Archive& Ar ) override;

		void UploadCachedShadowmap(D3D12CommandList* CmdList);

		inline float GetRadius() const { return m_Radius; }
		inline float GetMaxDrawDistance() const { return MaxDrawDistance; };
		Matrix GetLocalToWorld() const;

		void SetRadius( float Radius );
		void SetMaxDrawDistance( float InMaxDrawDistance );

		virtual void SetStatic( bool bInStatic ) override;
		virtual void SetCastStaticShadow( bool bInCastShadow ) override;

		inline bool CanUseStaticShadowmap() const { return IsStatic() && bCastStaticShadow; }
		inline TRefCountPtr<RenderTextureCube>& GetCachedShadowmap() { return CachedShadowmap; }
		inline StaticShadowDepthMapData& GetCachedShadowmapData() { return CachedShadowmapData; }

		void InvalidateCachedShadow() { CachedShadowmap = nullptr; CachedShadowmapData.Empty(); MarkRenderStateDirty(); }

#if WITH_EDITOR
		virtual void DrawDetailPanel(float DeltaTime) override;

		void DrawAttenuation();

		void DrawEditorDefault() override;
		void DrawEditorSelected() override;

		bool IsRequiredShadowBake() const { return bRequiredStaticShadowmapBake; }
		void ClearRequiredShadowBake() { bRequiredStaticShadowmapBake = false; }
#endif

	protected:

		virtual void RegisterComponent(World* InOwningWorld) override;
		virtual void UnRegisterComponent() override;

		virtual void OnUpdateTransform( bool SkipPhysic ) override;

		float m_Radius;
		float MaxDrawDistance;

		TRefCountPtr<class RenderTextureCube> CachedShadowmap;
		StaticShadowDepthMapData CachedShadowmapData;

		class PointLightSceneProxy* m_PointLightSceneProxy;

#if WITH_EDITOR
		inline virtual bool HasSprite() const override { return true; }

		bool bRequiredStaticShadowmapBake = false;
#endif

	private:
	};
}