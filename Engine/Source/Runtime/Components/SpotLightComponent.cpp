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
		, MaxDrawDistance(0.0f)
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
			Ar >> MaxDrawDistance;

			Ar >> CachedShadowmapData;
		}
		else
		{
			Ar << m_Attenuation;
			Ar << m_OuterRadius;
			Ar << m_InnerRadius;
			Ar << MaxDrawDistance;

			Ar << CachedShadowmapData;
		}
	}

	void SpotLightComponent::UploadCachedShadowmap( D3D12CommandList* CmdList )
	{
		if (!CachedShadowmap && CanUseStaticShadowmap() && !CachedShadowmapData.DepthSamples.empty())
		{
			drn_check(CachedShadowmapData.ShadowMapSizeX * CachedShadowmapData.ShadowMapSizeY == CachedShadowmapData.DepthSamples.size());

			RenderResourceCreateInfo TextureCreateInfo( CachedShadowmapData.DepthSamples.data(), nullptr, ClearValueBinding::Black, "SpotlightStaticShadowDepthmap" );
			CachedShadowmap = RenderTexture2D::Create(CmdList, CachedShadowmapData.ShadowMapSizeX, CachedShadowmapData.ShadowMapSizeY, DXGI_FORMAT_D16_UNORM, 1, 1, true,
				(ETextureCreateFlags)(ETextureCreateFlags::ShaderResource | ETextureCreateFlags::DepthStencilTargetable | ETextureCreateFlags::NoFastClear), TextureCreateInfo);
		}
	}

	void SpotLightComponent::SetMaxDrawDistance( float InMaxDrawDistance )
	{
		MaxDrawDistance = InMaxDrawDistance;
		if (m_SpotLightSceneProxy)
		{
			m_SpotLightSceneProxy->MaxDrawDistance = MaxDrawDistance;
		}
	}

	void SpotLightComponent::SetCastStaticShadow( bool bInCastShadow )
	{
		LightComponent::SetCastStaticShadow(bInCastShadow);

		if (!bCastStaticShadow)
		{
			InvalidateCachedShadow();
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
		if (m_SpotLightSceneProxy)
		{
			m_SpotLightSceneProxy->MarkPendingKill();

			m_SpotLightSceneProxy = nullptr;
			m_LightSceneProxy = nullptr;
		}


		LightComponent::UnRegisterComponent();
	}

	void SpotLightComponent::OnUpdateTransform( bool SkipPhysic )
	{
		LightComponent::OnUpdateTransform(SkipPhysic);
		MarkRenderStateDirty();
	}

	void SpotLightComponent::SetStatic( bool bInStatic )
	{
		LightComponent::SetStatic(bInStatic);

		if (!bInStatic) { InvalidateCachedShadow(); }
	}

#if WITH_EDITOR
	void SpotLightComponent::DrawDetailPanel( float DeltaTime )
	{
		LightComponent::DrawDetailPanel(DeltaTime);

		if (ImGui::Button("BakeShadowmap"))
		{
			if (CanUseStaticShadowmap())
			{
				InvalidateCachedShadow();
				bRequiredStaticShadowmapBake = true;
			}
		}

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

		if ( ImGui::SliderFloat( "Attenuation", &m_Attenuation, 0.05, 1000 ) )
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

		if ( ImGui::InputFloat( "MaxDrawDistance", &MaxDrawDistance ) )
		{
			SetMaxDrawDistance(MaxDrawDistance);
		}

// ---------------------------------------------------------------------------------------------------

		if ( ImGui::Checkbox( "CastStaticShadows", &bCastStaticShadow ) )
		{
			SetCastStaticShadow(bCastStaticShadow);
		}

		if ( ImGui::Checkbox( "CastDynamicShadows", &bCastDynamicShadow ) )
		{
			SetCastDynamicShadow(bCastDynamicShadow);
		}
	}

	void SpotLightComponent::DrawAttenuation()
	{
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

	void SpotLightComponent::DrawEditorDefault()
	{
		LightComponent::DrawEditorDefault();

		if (GetWorld() && GetWorld()->HasViewFlag(EWorldViewFlag::Light))
		{
			DrawAttenuation();
		}
	}

	void SpotLightComponent::DrawEditorSelected()
	{
		LightComponent::DrawEditorSelected();

		if (GetWorld() && !GetWorld()->HasViewFlag(EWorldViewFlag::Light))
		{
			DrawAttenuation();
		}
	}

#endif
}