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
		m_OwnerComponent = InOwnerComponent;
		m_BodySetup = Setup;

		if (!Setup)
		{
			return;
		}

		physx::PxPhysics* Physics = PhysicManager::Get()->GetPhysics();

		// TODO: cleanup memory
		m_Material = Physics->createMaterial( 0.5f, 0.5f, 0.6f );

		if (m_SimulatePhysic)
		{
			m_RigidActor = Physics->createRigidDynamic( Transform2P(m_OwnerComponent->GetWorldTransform()) );
		}

		else
		{
			m_RigidActor = Physics->createRigidStatic( Transform2P(m_OwnerComponent->GetWorldTransform()) );
		}

		if (Setup->m_UseTriMesh)
		{
			for (int32 i = 0; i < Setup->m_TriMeshes.size(); i++)
			{
				physx::PxShape* shape = Physics->createShape(PxTriangleMeshGeometry(Setup->m_TriMeshes[i].TriMesh), *m_Material);

				m_RigidActor->attachShape( *shape );
				PX_RELEASE(shape);
			}
		}

		else
		{
			for (int32 i = 0; i < Setup->m_AggGeo.GetElementCount(); i++)
			{
				ShapeElem* Element = Setup->m_AggGeo.GetElement(i);

				//PxFilterData filterData;
				//filterData.setToDefault();
				//filterData.word0 = UINT32_MAX;
				//filterData.word1 = UINT32_MAX;

				physx::PxShape* shape = Physics->createShape( *(Element->GetPxGeometery(m_OwnerComponent->GetWorldScale()).get()), *m_Material );
				//shape->setSimulationFilterData( filterData );

				shape->userData = &Element->GetUserData();

				if (Element->GetType() == EAggCollisionShape::Sphere)
				{
					SphereElem* SphereEl = Element->GetShape<SphereElem>();
					shape->setLocalPose(Transform2P( Transform(SphereEl->Center, Quat(), Vector::OneVector ) ) );
				}

				m_RigidActor->attachShape( *shape );
				shape->release();
			}
		}


		//m_RigidActor->

		//m_RigidActor->is<physx::PxRigidDynamic>()->setMassSpaceInertiaTensor(m_SimulatePhysic ? m_Mass : physx::PxReal(0));

		m_PhysicUserData = PhysicUserData(this);
		m_RigidActor->userData = &m_PhysicUserData;
		InScene->AddActor(m_RigidActor);
	}

	void BodyInstance::TermBody()
	{
		m_OwnerComponent = nullptr;

		if (m_RigidActor && m_RigidActor->getScene())
		{
			m_RigidActor->getScene()->removeActor(*m_RigidActor);
		}

		PX_RELEASE(m_RigidActor);
	}

	void BodyInstance::SetBodyTransform( const Transform& InTransform )
	{
		if (m_RigidActor)
		{
			m_RigidActor->setGlobalPose(Transform2P(InTransform));
		}
	}

	void BodyInstance::UpdateBodyScale( const Vector& InScale )
	{
		if (m_RigidActor)
		{
			physx::PxPhysics* Physics = PhysicManager::Get()->GetPhysics();

			std::vector<PxShape*> Shapes;
			GetAllShapes(Shapes);

			for (PxShape* Shape : Shapes)
			{
				Transform LocalTransform = P2Transform( Shape->getLocalPose() );
				ShapeElem* Elem = PhysicUserData::Get<ShapeElem>(Shape->userData);

				if (Elem->GetType() == EAggCollisionShape::Sphere)
				{
					SphereElem* sphereElem = Elem->GetShape<SphereElem>();

					PxGeometryHolder GeoHolder = Shape->getGeometry();
					PxSphereGeometry& Sphere = GeoHolder.sphere();

					float MaxAxis = std::max( std::max( InScale.GetX(), InScale.GetY() ), InScale.GetZ() );
					Sphere.radius = sphereElem->Radius * MaxAxis;

					if (Sphere.isValid())
					{
						Shape->setGeometry(Sphere);
					}

					// TODO: add translation after scale
				}

				else if (Elem->GetType() == EAggCollisionShape::Box)
				{
					BoxElem* boxElem = Elem->GetShape<BoxElem>();

					PxGeometryHolder GeoHolder = Shape->getGeometry();
					PxBoxGeometry& Box = GeoHolder.box();

					Box.halfExtents.x = boxElem->Extent.GetX() * InScale.GetX();
					Box.halfExtents.y = boxElem->Extent.GetY() * InScale.GetY();
					Box.halfExtents.z = boxElem->Extent.GetZ() * InScale.GetZ();

					if (Box.isValid())
					{
						Shape->setGeometry(Box);
					}

					// TODO: add translation after scale
				}
			}

		}
	}

	void BodyInstance::AddForce( const Vector& Force, bool AccelChange )
	{
		PxRigidBody* RigidBody = m_RigidActor ? m_RigidActor->is<PxRigidBody>() : nullptr;
		if (RigidBody)
		{
			RigidBody->addForce(Vector2P( Force ), AccelChange ? PxForceMode::eACCELERATION : PxForceMode::eFORCE);
		}
	}

	void BodyInstance::AddImpulse( const Vector& Impulse, bool AccelChange )
	{
		PxRigidBody* RigidBody = m_RigidActor ? m_RigidActor->is<PxRigidBody>() : nullptr;
		if (RigidBody)
		{
			RigidBody->addForce(Vector2P( Impulse ), AccelChange ? PxForceMode::eVELOCITY_CHANGE : PxForceMode::eIMPULSE);
		}
	}

	int32 BodyInstance::GetAllShapes( std::vector<PxShape*>& Result )
	{
		uint32 NumShapes = 0;

		if (m_RigidActor)
		{
			NumShapes = m_RigidActor->getNbShapes();
			Result.resize(NumShapes);

			m_RigidActor->getShapes(Result.data(), NumShapes);
		}

		return NumShapes;
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