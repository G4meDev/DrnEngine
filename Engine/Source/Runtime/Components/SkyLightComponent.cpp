#include "DrnPCH.h"
#include "SkyLightComponent.h"

#include "Runtime/Engine/SkyLightSceneProxy.h"

namespace Drn
{
	SkyLightComponent::SkyLightComponent()
		: LightComponent()
		, m_SkyLightSceneProxy(nullptr)
		, m_Cubemap("Cubemap", "NULL")
		, m_BlockLowerHemesphere(false)
		, m_LowerHemesphereColor()
	{
	}

	SkyLightComponent::~SkyLightComponent()
	{
	
	}

	void SkyLightComponent::Serialize( Archive& Ar )
	{
		LightComponent::Serialize(Ar);

		if (Ar.IsLoading())
		{
			m_Cubemap.Serialize(Ar);
			Ar >> m_BlockLowerHemesphere;
			Ar >> m_LowerHemesphereColor;

			m_Cubemap.m_TextureCube.LoadChecked();
		}
		else
		{
			m_Cubemap.Serialize(Ar);
			Ar << m_BlockLowerHemesphere;
			Ar << m_LowerHemesphereColor;
		}
	}

	void SkyLightComponent::SetCubemap( const AssetHandle<TextureCube>& Cubemap )
	{
		m_Cubemap.m_TextureCube = Cubemap;
		MarkRenderStateDirty();
	}

	void SkyLightComponent::SetBlockLowerHemisphere( bool Block )
	{
		m_BlockLowerHemesphere = Block;
		MarkRenderStateDirty();
	}

	void SkyLightComponent::SetLowerHemisphereColor( const Vector& InColor )
	{
		m_LowerHemesphereColor = InColor;
		MarkRenderStateDirty();
	}

	void SkyLightComponent::RegisterComponent( World* InOwningWorld )
	{
		LightComponent::RegisterComponent(InOwningWorld);

		m_SkyLightSceneProxy = new SkyLightSceneProxy(this);
		InOwningWorld->GetScene()->RegisterSkyLightProxy(m_SkyLightSceneProxy);
		m_LightSceneProxy = m_SkyLightSceneProxy;

#if WITH_EDITOR
		AssetHandle<Texture2D> PointLightIcon( "Engine\\Content\\EditorResources\\LightIcons\\T_SkyLightIcon.drn" );
		PointLightIcon.Load();

		m_Sprite->SetSprite( PointLightIcon );
#endif
	}

	void SkyLightComponent::UnRegisterComponent()
	{
		if (GetWorld())
		{
			GetWorld()->GetScene()->UnRegisterSkyLightProxy( m_SkyLightSceneProxy);
		}

		m_SkyLightSceneProxy = nullptr;
		m_LightSceneProxy = nullptr;

		LightComponent::UnRegisterComponent();
	}

	void SkyLightComponent::OnUpdateTransform( bool SkipPhysic )
	{
		LightComponent::OnUpdateTransform(SkipPhysic);
	}

#if WITH_EDITOR
	void SkyLightComponent::DrawDetailPanel( float DeltaTime )
	{
		LightComponent::DrawDetailPanel(DeltaTime);

		float Color[3] = { m_LightColor.GetX(), m_LightColor.GetY(), m_LightColor.GetZ() };
		if (ImGui::ColorEdit3("Color", Color))
		{
			SetColor( Vector(Color[0], Color[1], Color[2]) );
		}

		if ( ImGui::DragFloat( "Intensity", &m_Intensity, 0.005f , 0, 10 ) )
		{
			SetIntensity(m_Intensity);
		}

		if ( ImGui::Checkbox( "Block Lower Hemisphere", &m_BlockLowerHemesphere) )
		{
			SetBlockLowerHemisphere(m_BlockLowerHemesphere);
		}

		float CC[3] = { m_LowerHemesphereColor.GetX(), m_LowerHemesphereColor.GetY(), m_LowerHemesphereColor.GetZ() };
		if (ImGui::ColorEdit3("Lower Hemisphere Color", CC))
		{
			SetLowerHemisphereColor( Vector(CC[0], CC[1], CC[2]) );
		}

		AssetHandle<TextureCube> Cubemap = m_Cubemap.Draw();
		if (Cubemap.IsValid())
		{
			SetCubemap(Cubemap);
		}
	}
#endif
}