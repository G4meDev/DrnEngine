#include "DrnPCH.h"
#include "CharacterMovementComponent.h"

#if WITH_EDITOR
#include <imgui.h>
#endif

namespace Drn
{
	CharacterMovementComponent::CharacterMovementComponent()
		: Component()
		, m_Controller(nullptr)
		, m_MovementInput(Vector::ZeroVector)
		, m_HalfHeight(3.0f)
		, m_Radius(1.0f)
	{
	
	}

	CharacterMovementComponent::~CharacterMovementComponent()
	{
		
	}

	void CharacterMovementComponent::Tick( float DeltaTime )
	{
		Component::Tick(DeltaTime);

		if (m_Controller)
		{
			physx::PxControllerFilters Filter;
			Filter.mFilterFlags = PxQueryFlag::eSTATIC | PxQueryFlag::eDYNAMIC;

			m_Controller->move(Vector2P(m_MovementInput + Vector::DownVector * 0.1f), 0.001f, DeltaTime, Filter);
		}
	}

	void CharacterMovementComponent::Serialize( Archive& Ar )
	{
		Component::Serialize(Ar);

		if (Ar.IsLoading())
		{
			Ar >> m_HalfHeight;
			Ar >> m_Radius;
		}
		else
		{
			Ar << m_HalfHeight;
			Ar << m_Radius;
		}
	}


	void CharacterMovementComponent::RegisterComponent( World* InOwningWorld )
	{
		Component::RegisterComponent(InOwningWorld);

		// TODO: cleanup memory
		physx::PxMaterial* PMaterial = PhysicManager::Get()->GetPhysics()->createMaterial( 0.5f, 0.5f, 0.6f );

		physx::PxCapsuleControllerDesc Desc;
		Desc.material = PMaterial;
		Desc.upDirection = Vector2P(Vector::UpVector);
		Desc.position = Vector2Pd(CalculateCapsulePosition());
		Desc.height = CalculateRealHeight();
		Desc.radius = m_Radius;
		Desc.userData = &GetUserData();

		m_Controller = InOwningWorld->GetPhysicScene()->GetControllerManager()->createController(Desc);
	}
	
	void CharacterMovementComponent::UnRegisterComponent()
	{
		PX_RELEASE(m_Controller);

		Component::UnRegisterComponent();
	}

	void CharacterMovementComponent::SetMovementInput( const Vector& Input )
	{
		m_MovementInput = Input;
	}

	void CharacterMovementComponent::SendPhysicTranform( const Transform& InTransform )
	{
		if (m_Controller)
		{
			m_Controller->setFootPosition(Vector2Pd(InTransform.GetLocation()));
		}
	}

	Vector CharacterMovementComponent::CalculateCapsulePosition() const
	{
		return GetOwningActor()->GetActorLocation() + Vector::UpVector * m_HalfHeight;
	}

	float CharacterMovementComponent::CalculateRealHeight() const
	{
		return (m_HalfHeight - m_Radius) * 2;
	}

	void CharacterMovementComponent::SetHalfHeight( float HalfHeight )
	{
		m_HalfHeight = HalfHeight;
		if (m_Controller)
		{
			m_Controller->resize(CalculateRealHeight());
			m_Controller->setPosition(Vector2Pd(CalculateCapsulePosition()));
		}
	}

	void CharacterMovementComponent::SetRadius( float Radius )
	{
		m_Radius = Radius;
		PxCapsuleController* CC = static_cast<PxCapsuleController*>(m_Controller);
		if (CC)
		{
			CC->setRadius(Radius);
			CC->setPosition(Vector2Pd(CalculateCapsulePosition()));
		}
	}

#if WITH_EDITOR
	void CharacterMovementComponent::DrawDetailPanel( float DeltaTime )
	{
		Component::DrawDetailPanel(DeltaTime);

		if ( ImGui::DragFloat( "HalfHeight", &m_HalfHeight, 0.05, 0, 10, "%.2f" ) )
		{
			SetHalfHeight(m_HalfHeight);
		}

		if ( ImGui::DragFloat( "Radius", &m_Radius, 0.05, 0, 10, "%.2f" ) )
		{
			SetRadius(m_Radius);
		}
	}
#endif
}