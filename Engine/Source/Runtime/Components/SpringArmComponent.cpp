#include "DrnPCH.h"
#include "SpringArmComponent.h"

namespace Drn
{
	SpringArmComponent::SpringArmComponent()
		: SceneComponent()
		, m_ArmLength(5.0f)
		, m_LocationLag(false)
		, m_RotationLag(false)
	{
		
	}

	SpringArmComponent::~SpringArmComponent()
	{
		
	}

	void SpringArmComponent::Serialize( Archive& Ar )
	{
		SceneComponent::Serialize(Ar);

		if (Ar.IsLoading())
		{
			Ar >> m_LocationLag;
			Ar >> m_RotationLag;
			Ar >> m_ArmLength;
		}

		else
		{
			Ar << m_LocationLag;
			Ar << m_RotationLag;
			Ar << m_ArmLength;
		}
	}

	void SpringArmComponent::Tick( float DeltaTime )
	{
		SceneComponent::Tick(DeltaTime);

		UpdateDesiredLocationAndRotation(DeltaTime, m_LocationLag, m_RotationLag);
		UpdateChildTransforms();
	}

	void SpringArmComponent::OnUpdateTransform( bool SkipPhysic )
	{
		SceneComponent::OnUpdateTransform(SkipPhysic);

	}

	void SpringArmComponent::UpdateDesiredLocationAndRotation( float DeltaTime, bool LocationLag, bool RotationLag )
	{
		if (LocationLag)
		{

		}

		else
		{
			m_DesiredLocation = GetWorldLocation() + GetWorldRotation().GetVector() * -m_ArmLength;
		}

		if (RotationLag)
		{
			
		}

		else
		{
			m_DesiredRotation = GetWorldRotation();
		}
	}

	void SpringArmComponent::UpdateChildTransforms()
	{
		
	}

#if WITH_EDITOR
	void SpringArmComponent::DrawDetailPanel( float DeltaTime )
	{
		SceneComponent::DrawDetailPanel(DeltaTime);

	}

	void SpringArmComponent::DrawArm()
	{
		if (GetWorld())
		{
			GetWorld()->DrawDebugLine(GetWorldLocation(), m_DesiredLocation, Color::Red, 0, 0);
		}
	}

	void SpringArmComponent::DrawEditorDefault()
	{
		SceneComponent::DrawEditorDefault();

	}

	void SpringArmComponent::DrawEditorSelected()
	{
		SceneComponent::DrawEditorSelected();

		UpdateDesiredLocationAndRotation(0, false, false);
		DrawArm();
	}
#endif
}