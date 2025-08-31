#include "DrnPCH.h"
#include "DirectionalLightComponent.h"

#include "Runtime/Engine/DirectionalLightSceneProxy.h"

#define DIRECTIONAL_SHADOW_CASCADE_MAX 8

namespace Drn
{
	DirectionalLightComponent::DirectionalLightComponent()
		: LightComponent()
		, m_DirectionalLightSceneProxy(nullptr)
		, m_ShadowDistance(500.0f)
		, m_CascadeCount(4)
		, m_CascadeLogDistribution(0.65f)
		, m_CascadeDepthScale(1.5f)
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
			Ar >> m_CascadeCount;
			Ar >> m_ShadowDistance;
			Ar >> m_CascadeLogDistribution;
			Ar >> m_CascadeDepthScale;
		}

		else
		{
			Ar << m_CascadeCount;
			Ar << m_ShadowDistance;
			Ar << m_CascadeLogDistribution;
			Ar << m_CascadeDepthScale;
		}
	}

	void DirectionalLightComponent::SetCascadeCount( int32 CascadeCount )
	{
		m_CascadeCount = std::clamp(CascadeCount, 1, DIRECTIONAL_SHADOW_CASCADE_MAX);
		MarkRenderStateDirty();
	}

	void DirectionalLightComponent::SetShadowDistance( float ShadowDistance )
	{
		m_ShadowDistance = std::max(10.0f, ShadowDistance);
		MarkRenderStateDirty();
	}

	void DirectionalLightComponent::SetCascadeLogDistribution( float LogDistribution )
	{
		m_CascadeLogDistribution = std::clamp(LogDistribution, 0.0f, 1.0f);
		MarkRenderStateDirty();
	}

	void DirectionalLightComponent::SetCascadeDepthScale( float DepthScale )
	{
		m_CascadeDepthScale = DepthScale;
		MarkRenderStateDirty();
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
		MarkRenderStateDirty();
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

		if ( ImGui::DragFloat( "Shadow Distance", &m_ShadowDistance, 1.0f, 0.0f, 3000.0f, "%.0f" ) )
		{
			SetShadowDistance(m_ShadowDistance);
		}

		if ( ImGui::InputInt( "Cascade Count", &m_CascadeCount, 1, 1, m_CastShadow ? ImGuiInputTextFlags_::ImGuiInputTextFlags_None : ImGuiInputTextFlags_ReadOnly) )
		{
			SetCascadeCount(m_CascadeCount);
		}

		if ( ImGui::DragFloat( "Cascade Logarithmic Distribution", &m_CascadeLogDistribution, 0.05f, 0.0f, 1.0f, "%.2f" ) )
		{
			SetCascadeLogDistribution(m_CascadeLogDistribution);
		}

		if ( ImGui::DragFloat( "Cascade Depth Scale", &m_CascadeDepthScale, 0.1f, 1.0f, 10.0f, "%.1f" ) )
		{
			SetCascadeDepthScale(m_CascadeDepthScale);
		}

		if ( ImGui::InputFloat( "DepthBias", &m_DepthBias, 0.0001f, 1.0f, "%.4f" ) )
		{
			SetDepthBias(m_DepthBias);
		}
	}

	void DirectionalLightComponent::DrawEditorSelected()
	{
		LightComponent::DrawEditorSelected();

		if (GetWorld())
		{
			GetWorld()->DrawDebugArrow(GetWorldLocation(), GetWorldLocation() + GetWorldRotation().GetVector() * 1.2f, 0.1f, Color::White, 0.0f, 0.0f);
		}
	}

#endif
}