#include "DrnPCH.h"
#include "LightComponent.h"
#include "Runtime/Engine/LightSceneProxy.h"

namespace Drn
{
	LightComponent::LightComponent()
		: SceneComponent()
		, m_LightColor(Vector::OneVector)
		, m_Intensity(1.0f)
		, m_DepthBias(0.005)
		, m_CastShadow(false)
		, bCastStaticShadow(true)
		, bCastDynamicShadow(true)
		, m_RenderStateDirty(true)
	{
	}

	LightComponent::~LightComponent()
	{
	}

	void LightComponent::Serialize( Archive& Ar )
	{
		SceneComponent::Serialize(Ar);

		if (Ar.IsLoading())
		{
			Ar >> m_LightColor;
			Ar >> m_Intensity;
			Ar >> m_DepthBias;

			Ar >> m_CastShadow;
			Ar >> bCastStaticShadow;
			Ar >> bCastDynamicShadow;
		}

		else
		{
			Ar << m_LightColor;
			Ar << m_Intensity;
			Ar << m_DepthBias;

			Ar << m_CastShadow;
			Ar << bCastStaticShadow;
			Ar << bCastDynamicShadow;
		}
	}

	void LightComponent::RegisterComponent( World* InOwningWorld )
	{
		SceneComponent::RegisterComponent(InOwningWorld);


	}

	void LightComponent::UnRegisterComponent()
	{
		SceneComponent::UnRegisterComponent();

	}

	void LightComponent::OnUpdateTransform( bool SkipPhysic )
	{
		SceneComponent::OnUpdateTransform(SkipPhysic);
	}

	void LightComponent::SetColor( const Vector& Color )
	{
		m_LightColor = Color;
		MarkRenderStateDirty();
	}

	void LightComponent::SetIntensity( float Intensity )
	{
		m_Intensity = Intensity;
		MarkRenderStateDirty();
	}

	void LightComponent::SetCastShadow( bool CastShadow )
	{
		m_CastShadow = CastShadow;
		MarkRenderStateDirty();
	}

	void LightComponent::SetDepthBias( float DepthBias )
	{
		m_DepthBias = DepthBias;
		MarkRenderStateDirty();
	}

	void LightComponent::SetCastStaticShadow( bool bInCastShadow )
	{
		bCastStaticShadow = bInCastShadow;
		if (m_LightSceneProxy)
		{
			m_LightSceneProxy->SetCastStaticShadow(bInCastShadow);
		}
	}

	void LightComponent::SetCastDynamicShadow( bool bInCastShadow )
	{
		bCastDynamicShadow = bInCastShadow;
		if (m_LightSceneProxy)
		{
			m_LightSceneProxy->SetCastDynamicShadow(bInCastShadow);
		}
	}

	void LightComponent::SetStatic( bool bInStatic )
	{
		SceneComponent::SetStatic(bInStatic);
		if (m_LightSceneProxy)
		{
			m_LightSceneProxy->SetStatic(bInStatic);
		}
	}

#if WITH_EDITOR
	void LightComponent::DrawDetailPanel( float DeltaTime )
	{
		if (ImGui::Checkbox("Static", &bStatic))
		{
			SetStatic(bStatic);
		}

		SceneComponent::DrawDetailPanel(DeltaTime);
	}

	void LightComponent::SetSelectedInEditor( bool SelectedInEditor )
	{
		SceneComponent::SetSelectedInEditor(SelectedInEditor);


	}
#endif
}