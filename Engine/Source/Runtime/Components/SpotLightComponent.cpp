#include "DrnPCH.h"
#include "SpotLightComponent.h"

#include "Runtime/Engine/SpotLightSceneProxy.h"

namespace Drn
{
	SpotLightComponent::SpotLightComponent()
		: LightComponent()
		, m_SpotLightSceneProxy(nullptr)
		, m_Attenuation(10)
		, m_OuterRadius(45)
		, m_InnerRadius(0)
	{

	}

	SpotLightComponent::~SpotLightComponent()
	{
	
	}

	void SpotLightComponent::Serialize( Archive& Ar )
	{
		LightComponent::Serialize(Ar);

		if (Ar.IsLoading())
		{
			Ar >> m_Attenuation;
			Ar >> m_OuterRadius;
			Ar >> m_InnerRadius;
		}
		else
		{
			Ar << m_Attenuation;
			Ar << m_OuterRadius;
			Ar << m_InnerRadius;
		}
	}

	void SpotLightComponent::RegisterComponent( World* InOwningWorld )
	{
		LightComponent::RegisterComponent(InOwningWorld);

		m_SpotLightSceneProxy = new SpotLightSceneProxy(this);
		InOwningWorld->GetScene()->RegisterLightProxy(m_SpotLightSceneProxy);
		m_LightSceneProxy = m_SpotLightSceneProxy;

#if WITH_EDITOR
		AssetHandle<Texture2D> PointLightIcon( "Engine\\Content\\EditorResources\\LightIcons\\T_SpotLightIcon.drn" );
		PointLightIcon.Load();

		m_Sprite->SetSprite( PointLightIcon );
#endif
	}

	void SpotLightComponent::UnRegisterComponent()
	{
		if (GetWorld())
		{
			GetWorld()->GetScene()->UnRegisterLightProxy( m_SpotLightSceneProxy);
		}

		m_SpotLightSceneProxy = nullptr;
		m_LightSceneProxy = nullptr;

		LightComponent::UnRegisterComponent();
	}

	void SpotLightComponent::OnUpdateTransform( bool SkipPhysic )
	{
		LightComponent::OnUpdateTransform(SkipPhysic);
		MarkRenderStateDirty();
	}

#if WITH_EDITOR
	void SpotLightComponent::DrawDetailPanel( float DeltaTime )
	{
		LightComponent::DrawDetailPanel(DeltaTime);

		float Color[3] = { m_LightColor.GetX(), m_LightColor.GetY(), m_LightColor.GetZ() };
		if (ImGui::ColorEdit3("Color", Color))
		{
			SetColor( Vector(Color[0], Color[1], Color[2]) );
		}

		if ( ImGui::DragFloat( "Intensity", &m_Intensity, 0.1f, 0, 100 ) )
		{
			SetIntensity(m_Intensity);
		}

		if ( ImGui::Checkbox( "CastShadow", &m_CastShadow ) )
		{
			SetCastShadow(m_CastShadow);
		}

		if ( ImGui::SliderFloat( "Attenuation", &m_Attenuation, 0.05, 100 ) )
		{
			m_Attenuation = std::max(m_Attenuation, 0.0f);
			SetAttenuation(m_Attenuation);
		}

		if ( ImGui::SliderFloat( "OutterRadius", &m_OuterRadius, 0.05, 90 ) )
		{
			m_OuterRadius = std::max(m_OuterRadius, 0.0f);
			m_InnerRadius = std::clamp(m_InnerRadius, 0.0f, m_OuterRadius);
			SetOutterRadius(m_OuterRadius);
		}

		if ( ImGui::SliderFloat( "InnerRadius", &m_InnerRadius, 0, 90 ) )
		{
			m_InnerRadius = std::max(m_InnerRadius, 0.0f);
			m_OuterRadius = std::max(m_OuterRadius, m_InnerRadius);
			SetInnerRadius(m_InnerRadius);
			SetOutterRadius(m_OuterRadius);
		}
	
		if ( ImGui::InputFloat( "DepthBias", &m_DepthBias, 0.001f, 1.0f ) )
		{
			SetDepthBias(m_DepthBias);
		}
	}

	void SpotLightComponent::DrawEditorSelected()
	{
		LightComponent::DrawEditorSelected();

		if (GetWorld())
		{
			const float Inner = Math::DegreesToRadians(GetInnerRadius());
			const float Outer = Math::DegreesToRadians(GetOutterRadius());

			GetWorld()->DrawDebugCone(GetWorldLocation(), GetDirection(), GetAttenuation(), Outer, Outer, Color::White, 32, 0, 0);
			GetWorld()->DrawDebugConeCap(GetWorldLocation(), GetDirection(), GetAttenuation(), Outer, Color::White, 16, 0, 0);

			if ( GetInnerRadius() > 0 )
				GetWorld()->DrawDebugCone(GetWorldLocation(), GetDirection(), GetAttenuation(), Inner, Inner, Color::Blue, 32, 0, 0);
		}
	}

#endif
}