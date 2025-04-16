#pragma once

#include "ForwardTypes.h"

#include "Runtime/Physic/PhysicUserData.h"

#include <PxConfig.h>
#include <PxPhysics.h>
#include <PxPhysicsAPI.h>

LOG_DECLARE_CATEGORY(LogBodyInstance)

namespace Drn
{
	class BodyInstance
	{
	public:

		BodyInstance();
		~BodyInstance();

		inline PrimitiveComponent* GetOwnerComponent() { return m_OwnerComponent; }
		inline physx::PxRigidActor* GetRigidActor() { return m_RigidActor; }

		void InitBody(BodySetup* Setup, PrimitiveComponent* InOwnerComponent, PhysicScene* InScene);
		void TermBody();

	protected:

		PhysicUserData m_PhysicUserData;
		physx::PxRigidActor* m_RigidActor;
		PrimitiveComponent* m_OwnerComponent;
		physx::PxMaterial* m_Material;

	private:
	};
}