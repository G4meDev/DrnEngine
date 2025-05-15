#pragma once

#include "ForwardTypes.h"

#include "Runtime/Physic/PhysicUserData.h"

#include <PxConfig.h>
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

		inline BodySetup* GetBodySetup() const { return m_BodySetup; }

		void InitBody(BodySetup* Setup, PrimitiveComponent* InOwnerComponent, PhysicScene* InScene);
		void TermBody();

		void SetBodyTransform(const Transform& InTransform);
		void UpdateBodyScale(const Vector& InScale);

		void AddForce(const Vector& Force, bool AccelChange);
		void AddImpulse(const Vector& Impulse, bool AccelChange);


#if WITH_EDITOR
		void DrawDetailPanel(float DeltaTime);
#endif

	protected:

		int32 GetAllShapes( std::vector<PxShape*>& Result );

		PhysicUserData m_PhysicUserData;
		PxRigidActor* m_RigidActor;
		PrimitiveComponent* m_OwnerComponent;
		PxMaterial* m_Material;
		BodySetup* m_BodySetup;

		bool m_SimulatePhysic;
		float m_Mass;

	private:
		
	};
}