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
		, MaxDrawDistance(0.0f)
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
			Ar >> MaxDrawDistance;

			Ar >> CachedShadowmapData;
		}

		else
		{
			Ar << m_Radius;
			Ar << MaxDrawDistance;

			Ar << CachedShadowmapData;
		}
	}

	void PointLightComponent::UploadCachedShadowmap( D3D12CommandList* CmdList )
	{
		if (!CachedShadowmap && CanUseStaticShadowmap() && !CachedShadowmapData.DepthSamples.empty())
		{
			drn_check(CachedShadowmapData.ShadowMapSizeX * CachedShadowmapData.ShadowMapSizeY * 6 == CachedShadowmapData.DepthSamples.size());

			RenderResourceCreateInfo ShadowmapCubemapCreateInfo( CachedShadowmapData.DepthSamples.data(), nullptr, ClearValueBinding::DepthOne, "PointLightCachedShadowmap" );
			CachedShadowmap = RenderTextureCube::Create(CmdList, CachedShadowmapData.ShadowMapSizeX, DXGI_FORMAT_D16_UNORM, 1, 1, true,
				(ETextureCreateFlags)(ETextureCreateFlags::DepthStencilTargetable | ETextureCreateFlags::ShaderResource | ETextureCreateFlags::NoFastClear), ShadowmapCubemapCreateInfo);
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
		if (m_PointLightSceneProxy)
		{
			m_PointLightSceneProxy->MarkPendingKill();

			m_PointLightSceneProxy = nullptr;
			m_LightSceneProxy = nullptr;
		}


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

	void PointLightComponent::SetMaxDrawDistance( float InMaxDrawDistance )
	{
		MaxDrawDistance = InMaxDrawDistance;
		if (m_PointLightSceneProxy)
		{
			m_PointLightSceneProxy->MaxDrawDistance = MaxDrawDistance;
		}
	}

	void PointLightComponent::SetStatic( bool bInStatic )
	{
		LightComponent::SetStatic(bInStatic);

		if (!bInStatic)
		{
			InvalidateCachedShadow();
		}
	}

	void PointLightComponent::SetCastStaticShadow( bool bInCastShadow )
	{
		LightComponent::SetCastStaticShadow(bInCastShadow);

		if (!bInCastShadow)
		{
			InvalidateCachedShadow();
		}
	}

#if WITH_EDITOR
	void PointLightComponent::DrawDetailPanel( float DeltaTime )
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

		if ( ImGui::SliderFloat( "Radius", &m_Radius, 0.05, 1000 ) )
		{
			SetRadius(m_Radius);
		}
	
		if ( ImGui::InputFloat( "DepthBias", &m_DepthBias, 0.001f, 1.0f ) )
		{
			SetDepthBias(m_DepthBias);
		}

		if ( ImGui::InputFloat( "MaxDrawDistance", &MaxDrawDistance ) )
		{
			SetMaxDrawDistance(MaxDrawDistance);
		}

// --------------------------------------------------------------------------------

		if ( ImGui::Checkbox( "CastStaticShadows", &bCastStaticShadow ) )
		{
			SetCastStaticShadow(bCastStaticShadow);
		}

		if ( ImGui::Checkbox( "CastDynamicShadows", &bCastDynamicShadow ) )
		{
			SetCastDynamicShadow(bCastDynamicShadow);
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