#pragma once

#include "ForwardTypes.h"

#include "Runtime/Physic/PhysicUserData.h"

#include <PxConfig.h>
#include <PxPhysics.h>
#include <PxPhysicsAPI.h>

LOG_DECLARE_CATEGORY(LogBodyInstance)

namespace Drn
{
	class BodyInstance : public Serializable
	{
	public:

		BodyInstance();
		~BodyInstance();

		virtual void Serialize(Archive& Ar);

		inline PrimitiveComponent* GetOwnerComponent() { return m_OwnerComponent; }
		inline physx::PxRigidActor* GetRigidActor() { return m_RigidActor; }

		void InitBody(BodySetup* Setup, PrimitiveComponent* InOwnerComponent, PhysicScene* InScene);
		void TermBody();

#if WITH_EDITOR
		void DrawDetailPanel(float DeltaTime);
#endif

	protected:

		PhysicUserData m_PhysicUserData;
		physx::PxRigidActor* m_RigidActor;
		PrimitiveComponent* m_OwnerComponent;
		physx::PxMaterial* m_Material;

		bool m_SimulatePhysic;
		float m_Mass;

	private:
		
	};
}