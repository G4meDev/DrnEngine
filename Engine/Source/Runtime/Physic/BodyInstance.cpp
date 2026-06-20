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
		, bEnableGravity(true)
		, LinearDamping(0.0f)
		, AngularDamping(0.01f)
		, bNotifyOverlap(false)
		, bNotifyHit(false)
		, bUseCCD(false)
		, ObjectType(ECC_WorldStatic)
		, CollisionEnabled(ECollisionEnabled::QueryAndPhysics)
		, CollisionProfileName("BlockAll")
		, MaskFilter(0)
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

			uint8 CompactFlags = 0;
			Ar >> CompactFlags;

			bEnableGravity	= CompactFlags & (1 << 0);
			bNotifyOverlap	= CompactFlags & (1 << 1);
			bNotifyHit		= CompactFlags & (1 << 2);
			bUseCCD			= CompactFlags & (1 << 3);

			Ar >> LinearDamping;
			Ar >> AngularDamping;

			Ar >> CollisionProfileName;
			if (CollisionProfileName == CUSTOM_COLLISION_PROFILE_NAME)
			{
				Ar >> *(uint8*)&CollisionEnabled;
				Ar >> *(uint32*)&ObjectType;
				Ar >> ResponseToChannels;
			}
		}
		else
		{
			Ar << m_SimulatePhysic;
			Ar << m_Mass;

			uint8 CompactFlags = 0;
			CompactFlags |= (bEnableGravity << 0);
			CompactFlags |= (bNotifyOverlap << 1);
			CompactFlags |= (bNotifyHit		<< 2);
			CompactFlags |= (bUseCCD		<< 3);
			Ar << CompactFlags;

			Ar << LinearDamping;
			Ar << AngularDamping;

			Ar << CollisionProfileName;
			if (CollisionProfileName == CUSTOM_COLLISION_PROFILE_NAME)
			{
				Ar << (uint8)CollisionEnabled;
				Ar << ObjectType;
				Ar << ResponseToChannels;
			}
		}
	}

	void BodyInstance::InitBody( BodySetup* Setup, const Transform& BodyTransform, PrimitiveComponent* InOwnerComponent, PhysicScene* InScene )
	{
		m_OwnerComponent = InOwnerComponent;
		m_BodySetup = Setup;

		if (!Setup)
		{
			return;
		}

		physx::PxPhysics* Physics = PhysicManager::Get()->GetPhysics();

		// TODO: cleanup memory
		//m_Material = Physics->createMaterial( 0.5f, 0.5f, 0.6f );
		m_Material = Physics->createMaterial( 1.0f, 1.0f, 0.0f );

		if (m_SimulatePhysic)
		{
			physx::PxRigidDynamic* DynamicActor = Physics->createRigidDynamic( Transform2P(BodyTransform) );
			m_RigidActor = DynamicActor;

			DynamicActor->setSolverIterationCounts(8, 2);

			// TODO: cache center of mass and inertia instead of generating at runtime 
			//physx::PxRigidBodyExt::setMassAndUpdateInertia(*DynamicActor, m_Mass);
		}

		else
		{
			m_RigidActor = Physics->createRigidStatic( Transform2P(BodyTransform) );
		}

		if (Setup->m_UseTriMesh)
		{
			for (int32 i = 0; i < Setup->m_TriMeshes.size(); i++)
			{
				physx::PxShape* shape = Physics->createShape(PxTriangleMeshGeometry(Setup->m_TriMeshes[i].TriMesh, Vector2P(BodyTransform.GetScale())), *m_Material);
				shape->userData = &Setup->m_TriMeshes[i].GetUserData();

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

				physx::PxShape* shape = Physics->createShape( *(Element->GetPxGeometery(BodyTransform.GetScale()).get()), *m_Material );
				//shape->setSimulationFilterData( filterData );

				shape->userData = &Element->GetUserData();

				if (Element->GetType() == EAggCollisionShape::Sphere)
				{
					SphereElem* SphereEl = Element->GetShape<SphereElem>();
					shape->setLocalPose(Transform2P( Transform(SphereEl->Center, Quat(), Vector::OneVector ) ) );
				}

				// TODO: add rotation
				else if (Element->GetType() == EAggCollisionShape::Box)
				{
					BoxElem* BoxEl = Element->GetShape<BoxElem>();
					shape->setLocalPose(Transform2P( Transform(BoxEl->Center, Quat(), Vector::OneVector ) ) );
				}

				m_RigidActor->attachShape( *shape );
				shape->release();
			}
		}

		if (m_SimulatePhysic)
		{
			PxRigidDynamic* RigidDynamic = m_RigidActor ? m_RigidActor->is<PxRigidDynamic>() : nullptr;
			drn_check(RigidDynamic);

			// TODO: cache center of mass and inertia instead of generating at runtime 
			physx::PxRigidBodyExt::setMassAndUpdateInertia(*RigidDynamic, m_Mass);
		}

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

				if ( ShapeElem* Elem = PhysicUserData::Get<ShapeElem>( Shape->userData ) )
				{
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

				else if ( TriMeshGeom* Elem = PhysicUserData::Get<TriMeshGeom>( Shape->userData ) )
				{
					PxGeometryHolder NewGeom = Shape->getGeometry();
					PxTriangleMeshGeometry& Tri = NewGeom.triangleMesh();
					Tri.scale = Vector2P(InScale);

					if (Tri.isValid())
					{
						Shape->setGeometry(Tri);
					}
					
				}

			}

		}
	}

	Vector BodyInstance::GetCenterOfMass() const
	{
		PxRigidBody* RigidBody = m_RigidActor ? m_RigidActor->is<PxRigidBody>() : nullptr;
		if (RigidBody)
		{
			const physx::PxTransform LocalCOM = RigidBody->getCMassLocalPose();
			return P2Vector(RigidBody->getGlobalPose().transform(LocalCOM.p));
		}

		return Vector::ZeroVector;
	}

	float BodyInstance::GetMass() const
	{
		return m_Mass;
	}

	Vector BodyInstance::GetVelocity() const
	{
		PxRigidBody* RigidBody = m_RigidActor ? m_RigidActor->is<PxRigidBody>() : nullptr;
		if (RigidBody)
		{
			const physx::PxTransform LocalCOM = RigidBody->getCMassLocalPose();
			return P2Vector(RigidBody->getLinearVelocity());
		}

		return Vector::ZeroVector;
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

	void BodyInstance::AddTorque( const Vector& Force, bool AccelChange )
	{
		PxRigidBody* RigidBody = m_RigidActor ? m_RigidActor->is<PxRigidBody>() : nullptr;
		if (RigidBody)
		{
			RigidBody->addTorque(Vector2P( Force ), AccelChange ? PxForceMode::eACCELERATION : PxForceMode::eFORCE);
		}
	}

	void BodyInstance::AddForceAtPosition( const Vector& Force, const Vector& Position, bool AccelChange )
	{
		PxRigidBody* RigidBody = m_RigidActor ? m_RigidActor->is<PxRigidBody>() : nullptr;
		if (RigidBody)
		{
			physx::PxRigidBodyExt::addForceAtPos(*RigidBody, Vector2P( Force ), Vector2P( Position ), AccelChange ? PxForceMode::eACCELERATION : PxForceMode::eFORCE);
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
		if (ImGui::CollapsingHeader("Physic"))
		{
			ImGui::Checkbox( "Simulate Physic", &m_SimulatePhysic );
			ImGui::InputFloat( "Mass", &m_Mass);
			ImGui::InputFloat( "LinearDamping", &LinearDamping);
			ImGui::InputFloat( "AngularDamping", &AngularDamping);
			ImGui::Checkbox( "Enable Gravity", &bEnableGravity);
		}

		if (ImGui::CollapsingHeader("Collision"))
		{
			ImGui::Checkbox( "Generate Hit Event", &bNotifyHit);
			ImGui::Checkbox( "Generate Overlap Event", &bNotifyOverlap);
			ImGui::Checkbox( "Use CCD", &bUseCCD);

			DrawCollisionProfile("Profile", CollisionProfileName);

			if (CollisionProfileName == CUSTOM_COLLISION_PROFILE_NAME)
			{
				DrawCollisionProfileCustom(CollisionEnabled, ObjectType, ResponseToChannels);
			}
			else
			{
				DrawCollisionProfilePreset(CollisionProfileName);
			}
		}

		ImGui::Separator();
	}


#endif
}