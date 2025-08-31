#include "DrnPCH.h"
#include "SpringArmComponent.h"

namespace Drn
{
	SpringArmComponent::SpringArmComponent()
		: SceneComponent()
		, m_ArmLength(5.0f)
		, m_LocationLag(false)
		, m_RotationLag(false)
		, m_RelativeSocketLocation(Vector::ZeroVector)
		, m_RelativeSocketRotation(Quat::Identity)
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
	}

	void SpringArmComponent::OnUpdateTransform( bool SkipPhysic )
	{
		SceneComponent::OnUpdateTransform(SkipPhysic);

	}

	Transform SpringArmComponent::GetSocketTransform( const std::string& SocketName, ERelativeTransformSpace TransformSpace ) const
	{
		Transform RelativeTransform = Transform(m_RelativeSocketLocation, m_RelativeSocketRotation);

		switch ( TransformSpace )
		{
			// TODO: implement
			case ERelativeTransformSpace::Actor:
				
			case ERelativeTransformSpace::Component:
				return RelativeTransform;
			default:
				//return RelativeTransform * GetWorldTransform();
				return GetWorldTransform() * RelativeTransform;
		}
	}

	void SpringArmComponent::UpdateDesiredLocationAndRotation( float DeltaTime, bool LocationLag, bool RotationLag )
	{
		if (LocationLag)
		{

		}

		else
		{
			//m_DesiredLocation = GetWorldLocation() + GetWorldRotation().GetVector() * -m_ArmLength;
			m_RelativeSocketLocation = GetWorldRotation().GetVector() * -m_ArmLength;
		}

		if (RotationLag)
		{
			
		}

		else
		{
			m_DesiredRotation = GetWorldRotation();
		}

		UpdateChildTransforms(true);
	}

#if WITH_EDITOR
	void SpringArmComponent::DrawDetailPanel( float DeltaTime )
	{
		SceneComponent::DrawDetailPanel(DeltaTime);

		ImGui::DragFloat( "ArmLength", &m_ArmLength, 0.1f, 0.0f, 100.0f, "%.1f" );

		ImGui::Checkbox( "LocationLag", &m_LocationLag );
		ImGui::Checkbox( "RotationLag", &m_RotationLag );
	}

	void SpringArmComponent::DrawArm()
	{
		if (GetWorld())
		{
			GetWorld()->DrawDebugLine(GetWorldLocation(), GetSocketTransform("").GetLocation(), Color::Red, 0, 0);
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