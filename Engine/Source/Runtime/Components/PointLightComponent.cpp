#include "DrnPCH.h"
#include "PointLightComponent.h"
#include "Runtime/Engine/PointLightSceneProxy.h"

#if WITH_EDITOR
#include <imgui.h>
#endif

namespace Drn
{
	PointLightComponent::PointLightComponent()
		: LightComponent()
		, m_Radius(3.0f)
		, m_PointLightSceneProxy(nullptr)
	{
		m_Intensity = 35;
	}

	PointLightComponent::~PointLightComponent()
	{
		
	}

	void PointLightComponent::Serialize( Archive& Ar )
	{
		LightComponent::Serialize(Ar);

		if (Ar.IsLoading())
		{
			Ar >> m_Radius;
		}

		else
		{
			Ar << m_Radius;
		}
	}

	Matrix PointLightComponent::GetLocalToWorld() const
	{
		return Transform( GetWorldLocation(), Quat::Identity, GetRadius() );
	}

	void PointLightComponent::RegisterComponent( World* InOwningWorld )
	{
		LightComponent::RegisterComponent(InOwningWorld);

		m_PointLightSceneProxy = new PointLightSceneProxy(this);
		InOwningWorld->GetScene()->RegisterLightProxy(m_PointLightSceneProxy);
		m_LightSceneProxy = m_PointLightSceneProxy;

#if WITH_EDITOR
		AssetHandle<Texture2D> PointLightIcon( "Engine\\Content\\EditorResources\\LightIcons\\T_PointLightIcon.drn" );
		PointLightIcon.Load();

		m_Sprite->SetSprite( PointLightIcon );
#endif
	}

	void PointLightComponent::UnRegisterComponent()
	{
		if (GetWorld())
		{
			GetWorld()->GetScene()->UnRegisterLightProxy( m_PointLightSceneProxy );
		}

		m_PointLightSceneProxy = nullptr;
		m_LightSceneProxy = nullptr;

		LightComponent::UnRegisterComponent();
	}

	void PointLightComponent::OnUpdateTransform( bool SkipPhysic )
	{
		LightComponent::OnUpdateTransform(SkipPhysic);

		SetRelativeRotation(Quat::Identity);
		SetRelativeScale(Vector::OneVector);

		MarkRenderStateDirty();
	}

	void PointLightComponent::SetRadius( float Radius )
	{
		m_Radius = Radius;
		MarkRenderStateDirty();
	}

#if WITH_EDITOR
	void PointLightComponent::DrawDetailPanel( float DeltaTime )
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

		if ( ImGui::SliderFloat( "Radius", &m_Radius, 0.05, 100 ) )
		{
			SetRadius(m_Radius);
		}
	
		if ( ImGui::InputFloat( "DepthBias", &m_DepthBias, 0.001f, 1.0f ) )
		{
			SetDepthBias(m_DepthBias);
		}
	}

	void PointLightComponent::DrawAttenuation()
	{
		if (GetWorld())
		{
			GetWorld()->DrawDebugSphere( GetWorldLocation(), Quat::Identity, Color::White, GetRadius(), 36, 0.0, 0 );
		}
	}

	void PointLightComponent::DrawEditorDefault()
	{
		LightComponent::DrawEditorDefault();

		if (GetWorld() && GetWorld()->HasViewFlag(EWorldViewFlag::Light))
		{
			DrawAttenuation();
		}
	}

	void PointLightComponent::DrawEditorSelected()
	{
		LightComponent::DrawEditorSelected();

		if (GetWorld() && !GetWorld()->HasViewFlag(EWorldViewFlag::Light))
		{
			DrawAttenuation();
		}
	}

#endif
}