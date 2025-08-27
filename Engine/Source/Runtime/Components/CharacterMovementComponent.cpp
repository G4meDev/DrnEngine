#include "DrnPCH.h"
#include "CharacterMovementComponent.h"

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
			
		}
		else
		{

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
		Desc.position = Vector2Pd(GetOwningActor()->GetActorLocation());
		Desc.height = (m_HalfHeight - m_Radius) * 2;
		Desc.radius = 1.0f;
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

#if WITH_EDITOR
	void CharacterMovementComponent::DrawDetailPanel( float DeltaTime )
	{
		Component::DrawDetailPanel(DeltaTime);


	}
#endif
}