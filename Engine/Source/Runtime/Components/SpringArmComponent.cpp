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
		, m_LocationLagSpeed(3.0f)
		, m_RotationLagSpeed(3.0f)
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
			Ar >> m_ArmLength;
			Ar >> m_LocationLag;
			Ar >> m_LocationLagSpeed;
			Ar >> m_RotationLag;
			Ar >> m_RotationLagSpeed;
		}

		else
		{
			Ar << m_ArmLength;
			Ar << m_LocationLag;
			Ar << m_LocationLagSpeed;
			Ar << m_RotationLag;
			Ar << m_RotationLagSpeed;
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
				return RelativeTransform * GetWorldTransform();
		}
	}

	void SpringArmComponent::RegisterComponent( World* InOwningWorld )
	{
		SceneComponent::RegisterComponent(InOwningWorld);

		// TODO: move to begin play event
		UpdateDesiredLocationAndRotation(0, false, false);
	}

	void SpringArmComponent::UpdateDesiredLocationAndRotation( float DeltaTime, bool LocationLag, bool RotationLag )
	{
		Quat DesireRotation = GetWorldRotation();
		if (RotationLag)
		{
			DesireRotation = Math::QInterpTo(m_PreviousDesiredRotation, DesireRotation, DeltaTime, m_RotationLagSpeed);
		}
		
		m_PreviousDesiredRotation = DesireRotation;

		Vector DesireLocation = GetWorldTransform().TransformPosition(Vector(0, 0, -m_ArmLength));
		if (LocationLag)
		{
			DesireLocation = Math::VInterpTo(m_PreviousDesiredLocation, DesireLocation, DeltaTime, m_LocationLagSpeed);
		}

		m_PreviousDesiredLocation = DesireLocation;

		Transform RelTransform = Transform(DesireLocation, DesireRotation).GetRelativeTransform(GetWorldTransform());
		m_RelativeSocketLocation = RelTransform.GetLocation();
		m_RelativeSocketRotation = RelTransform.GetRotation();

		UpdateChildTransforms(true);
	}

#if WITH_EDITOR
	void SpringArmComponent::DrawDetailPanel( float DeltaTime )
	{
		SceneComponent::DrawDetailPanel(DeltaTime);

		ImGui::DragFloat( "ArmLength", &m_ArmLength, 0.1f, 0.0f, 100.0f, "%.1f" );

		ImGui::Checkbox( "LocationLag", &m_LocationLag );

		if (m_LocationLag)
		{
			ImGui::DragFloat("LocationLagSpeed", &m_LocationLagSpeed, 0.01f, 0.01f, 10.0f, "%.2f");
		}

		ImGui::Checkbox( "RotationLag", &m_RotationLag );

		if (m_RotationLag)
		{
			ImGui::DragFloat("RotationLagSpeed", &m_RotationLagSpeed, 0.01f, 0.01f, 10.0f, "%.2f");
		}
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