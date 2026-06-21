#pragma once

#include "ForwardTypes.h"

#include "Runtime/Physic/PhysicUserData.h"

#include <PxPhysics.h>
#include <PxPhysicsAPI.h>

LOG_DECLARE_CATEGORY(LogBodyInstance)

using namespace physx;

namespace Drn
{
	class BodyInstance : public Serializable
	{
	public:

		BodyInstance();
		~BodyInstance();

		virtual void Serialize(Archive& Ar);

		inline PrimitiveComponent* GetOwnerComponent() const { return m_OwnerComponent; }
		inline PxRigidActor* GetRigidActor() { return m_RigidActor; }
		inline bool IsValid() const { return m_RigidActor != nullptr; }

		inline BodySetup* GetBodySetup() const { return m_BodySetup; }

		void InitBody(BodySetup* Setup, const Transform& BodyTransform, PrimitiveComponent* InOwnerComponent, PhysicScene* InScene);
		void TermBody();

		void SetBodyTransform(const Transform& InTransform);
		void UpdateBodyScale(const Vector& InScale);

		Vector GetCenterOfMass() const;
		float GetMass() const;

		Vector GetVelocity() const;

		void AddForce(const Vector& Force, bool AccelChange);
		void AddImpulse(const Vector& Impulse, bool AccelChange);
		void AddTorque(const Vector& Force, bool AccelChange);

		void AddForceAtPosition(const Vector& Force, const Vector& Position, bool AccelChange);
		//void AddForceAtLocalPosition(const Vector& Force, const Vector& Position, bool AccelChange);

		void SetPhysicalMaterial(AssetHandle<PhysicalMaterial> InMaterial);

#if WITH_EDITOR
		void DrawDetailPanel(float DeltaTime);
		void DrawPhysicalMaterial();
#endif

	protected:

		int32 GetAllShapes( std::vector<PxShape*>& Result );

		PhysicUserData m_PhysicUserData;
		PxRigidActor* m_RigidActor;
		PrimitiveComponent* m_OwnerComponent;
		BodySetup* m_BodySetup;

		bool m_SimulatePhysic;
		float m_Mass;

		bool bEnableGravity;
		float LinearDamping;
		float AngularDamping;

		bool bNotifyOverlap;
		bool bNotifyHit;
		bool bUseCCD;

		std::string CollisionProfileName;
		ECollisionEnabled CollisionEnabled;
		ECollisionChannel ObjectType;
		CollisionResponseContainer ResponseToChannels;
		uint8 MaskFilter;
		AssetHandle<PhysicalMaterial> PhysicMaterial;

	private:
		
	};
}