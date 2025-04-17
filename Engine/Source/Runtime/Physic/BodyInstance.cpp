#include "DrnPCH.h"
#include "BodyInstance.h"

#include "Runtime/Physic/PhysicManager.h"

#if WITH_EDITOR
#include <imgui.h>
#endif

LOG_DEFINE_CATEGORY( LogBodyInstance, "BodyInstance" )

namespace Drn
{
	BodyInstance::BodyInstance()
		: m_RigidActor(nullptr)
		, m_OwnerComponent(nullptr)
		, m_SimulatePhysic(false)
		, m_Mass(10.0f)
	{
		
	}

	BodyInstance::~BodyInstance()
	{
		
	}

	void BodyInstance::Serialize( Archive& Ar )
	{
		if (Ar.IsLoading())
		{
			Ar >> m_SimulatePhysic;
			Ar >> m_Mass;
		}
		else
		{
			Ar << m_SimulatePhysic;
			Ar << m_Mass;
		}
	}

	void BodyInstance::InitBody( BodySetup* Setup, PrimitiveComponent* InOwnerComponent, PhysicScene* InScene )
	{
		LOG(LogBodyInstance, Info, "initalizing body for %s", InOwnerComponent->GetComponentLabel().c_str());

		m_OwnerComponent = InOwnerComponent;

		physx::PxPhysics* Physics = PhysicManager::Get()->GetPhysics();
		m_Material = Physics->createMaterial( 0.5f, 0.5f, 0.6f );

		// TODO: Add body setup
		physx::PxShape* shape = Physics->createShape( physx::PxBoxGeometry( Vector2P(m_OwnerComponent->GetWorldScale()) ), *m_Material );


		if (m_SimulatePhysic)
		{
			m_RigidActor = Physics->createRigidDynamic( Transform2P(m_OwnerComponent->GetWorldTransform()) );
		}

		else
		{
			m_RigidActor = Physics->createRigidStatic( Transform2P(m_OwnerComponent->GetWorldTransform()) );
		}

		m_RigidActor->attachShape( *shape );
		//m_RigidActor->setGlobalPose( Transform2P(m_OwnerComponent->GetWorldTransform()));
		//m_RigidActor->is<physx::PxRigidDynamic>()->setMassSpaceInertiaTensor(m_SimulatePhysic ? m_Mass : physx::PxReal(0));

		m_PhysicUserData = PhysicUserData(this);
		m_RigidActor->userData = &m_PhysicUserData;
		InScene->AddActor(m_RigidActor);

		shape->release();
	}

	void BodyInstance::TermBody()
	{
		LOG(LogBodyInstance, Info, "terminating body for %s", m_OwnerComponent->GetComponentLabel().c_str());

		m_OwnerComponent = nullptr;

		if (m_RigidActor->getScene())
		{
			m_RigidActor->getScene()->removeActor(*m_RigidActor);
		}

		PX_RELEASE(m_RigidActor);
	}

#if WITH_EDITOR
	void BodyInstance::DrawDetailPanel( float DeltaTime )
	{
		ImGui::Checkbox( "Simulate Physic", &m_SimulatePhysic );
		ImGui::InputFloat( "Mass", &m_Mass);

		ImGui::Separator();
	}
#endif

}