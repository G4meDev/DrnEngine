#pragma once

#include "ForwardTypes.h"

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

		inline physx::PxRigidActor* GetRigidActor() { return m_RigidActor; }

		void InitBody(BodySetup* Setup, PrimitiveComponent* InOwnerComponent, PhysicScene* InScene);

		void TermBody();

	protected:

		physx::PxRigidActor* m_RigidActor;

		PrimitiveComponent* m_OwnerComponent;

		physx::PxMaterial* m_Material;

	private:
	};
}