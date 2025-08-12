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
		Vector GetScaledColor() { return m_LightColor * m_Intensity; }

		void SetColor( const Vector& Color );
		void SetIntensity( float Intensity );
		void SetCastShadow( bool CastShadow );

		inline bool IsRenderStateDirty() const { return m_RenderStateDirty; }
		inline void ClearRenderStateDirty() { m_RenderStateDirty = false; }
		inline void MarkRenderStateDirty() { m_RenderStateDirty = true; }

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
		bool m_CastShadow;

		bool m_RenderStateDirty;

		class LightSceneProxy* m_LightSceneProxy;

	private:
	};
}