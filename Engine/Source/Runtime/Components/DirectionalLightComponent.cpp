#include "DrnPCH.h"
#include "DirectionalLightComponent.h"

#include "Runtime/Engine/DirectionalLightSceneProxy.h"

namespace Drn
{
	DirectionalLightComponent::DirectionalLightComponent()
		: LightComponent()
		, m_DirectionalLightSceneProxy(nullptr)
		, m_DepthBias(0.005)
	{
		
	}

	DirectionalLightComponent::~DirectionalLightComponent()
	{
		
	}

	void DirectionalLightComponent::Serialize( Archive& Ar )
	{
		LightComponent::Serialize(Ar);

		if (Ar.IsLoading())
		{
			Ar >> m_DepthBias;
		}

		else
		{
			Ar << m_DepthBias;
		}
	}

	void DirectionalLightComponent::SetDepthBias( float Bias )
	{
		m_DepthBias = Bias;
	}

	void DirectionalLightComponent::RegisterComponent( World* InOwningWorld )
	{
		LightComponent::RegisterComponent(InOwningWorld);

		m_DirectionalLightSceneProxy = new DirectionalLightSceneProxy(this);
		InOwningWorld->GetScene()->RegisterLightProxy(m_DirectionalLightSceneProxy);
		m_LightSceneProxy = m_DirectionalLightSceneProxy;

#if WITH_EDITOR
		AssetHandle<Texture2D> DirectionalLightIcon( "Engine\\Content\\EditorResources\\LightIcons\\T_DirectionalLightIcon.drn" );
		DirectionalLightIcon.Load();
		
		m_Sprite->SetSprite( DirectionalLightIcon );
#endif
	}

	void DirectionalLightComponent::UnRegisterComponent()
	{
		if (GetWorld())
		{
			GetWorld()->GetScene()->UnRegisterLightProxy( m_DirectionalLightSceneProxy);
		}

		m_DirectionalLightSceneProxy = nullptr;
		m_LightSceneProxy = nullptr;

		LightComponent::UnRegisterComponent();
	}

	void DirectionalLightComponent::OnUpdateTransform( bool SkipPhysic )
	{
		LightComponent::OnUpdateTransform(SkipPhysic);

	}

#if WITH_EDITOR
	void DirectionalLightComponent::DrawDetailPanel( float DeltaTime )
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

		if ( ImGui::InputFloat( "DepthBias", &m_DepthBias, 0.001f, 1.0f ) )
		{
			SetDepthBias(m_DepthBias);
		}
	}
#endif
}