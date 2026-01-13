#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class LightComponent : public SceneComponent
	{
	public:

		inline const Vector& GetLightColor() const { return m_LightColor; }
		inline float GetIntensity() const { return m_Intensity; }
		inline bool IsCastingShadow() const { return m_CastShadow; }
		inline float GetDepthBias() const { return m_DepthBias; }
		Vector GetScaledColor() { return m_LightColor * m_Intensity; }

		void SetColor( const Vector& Color );
		void SetIntensity( float Intensity );
		void SetCastShadow( bool CastShadow );
		void SetDepthBias( float DepthBias );

		inline bool IsRenderStateDirty() const { return m_RenderStateDirty; }
		inline void ClearRenderStateDirty() { m_RenderStateDirty = false; }
		inline void MarkRenderStateDirty() { m_RenderStateDirty = true; }

		virtual void SetStatic(bool bInStatic) override;

		void SetCastStaticShadow( bool bInCastShadow );
		void SetCastDynamicShadow( bool bInCastShadow );

		inline bool IsCastingStaticShadow() const { return bCastStaticShadow; }
		inline bool IsCastingDynamicShadow() const { return bCastDynamicShadow; }

	protected:
		LightComponent();
		virtual ~LightComponent();

		virtual void Serialize( Archive& Ar ) override;

		virtual void RegisterComponent(World* InOwningWorld) override;
		virtual void UnRegisterComponent() override;

		virtual void OnUpdateTransform( bool SkipPhysic ) override;

#if WITH_EDITOR
		virtual void DrawDetailPanel(float DeltaTime) override;
		virtual void SetSelectedInEditor( bool SelectedInEditor ) override;
#endif

		Vector m_LightColor;
		float m_Intensity;
		float m_DepthBias;

		// TODO: use bitfield for less memory footprint
		bool m_CastShadow;
		bool bCastStaticShadow;
		bool bCastDynamicShadow;

		bool m_RenderStateDirty;
		class LightSceneProxy* m_LightSceneProxy;

	private:
	};
}