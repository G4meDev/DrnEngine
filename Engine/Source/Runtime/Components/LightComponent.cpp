#include "DrnPCH.h"
#include "LightComponent.h"
#include "Runtime/Engine/LightSceneProxy.h"

namespace Drn
{
	LightComponent::LightComponent()
		: SceneComponent()
		, m_LightColor(Vector::OneVector)
		, m_Intensity(1.0f)
		, m_CastShadow(false)
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
			Ar >> m_CastShadow;
		}

		else
		{
			Ar << m_LightColor;
			Ar << m_Intensity;
			Ar << m_CastShadow;
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
		if (m_LightSceneProxy)
		{
			m_LightSceneProxy->SetColor(GetScaledColor());
		}
	}

	void LightComponent::SetIntensity( float Intensity )
	{
		m_Intensity = Intensity;
		if (m_LightSceneProxy)
		{
			m_LightSceneProxy->SetColor(GetScaledColor());
		}
	}

	void LightComponent::SetCastShadow( bool CastShadow )
	{
		m_CastShadow = CastShadow;
		if (m_LightSceneProxy)
		{
			m_LightSceneProxy->SetCastShadow( CastShadow );
		}
	}

	void LightComponent::DrawDetailPanel( float DeltaTime )
	{
		SceneComponent::DrawDetailPanel(DeltaTime);

		if ( m_LightSceneProxy )
		{
			m_LightSceneProxy->SetWorldPosition( GetWorldLocation() );
		}
	}

}