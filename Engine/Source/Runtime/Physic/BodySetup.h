#pragma once

#include "ForwardTypes.h"
#include "AggregateGeom.h"

#include <PxPhysics.h>
#include <PxPhysicsAPI.h>

namespace Drn
{
	class TriMeshGeom : public Serializable
	{
	public:

		TriMeshGeom()
			: TriMesh(nullptr)
		{};

		~TriMeshGeom() { ReleaseTriMesh(); }
		void ReleaseTriMesh() { PX_RELEASE(TriMesh); }

		inline PhysicUserData& GetUserData()
		{
			PhysicUserData::Set<TriMeshGeom>((void*)&UserData, const_cast<TriMeshGeom*>(this));
			return UserData;
		}

		virtual void Serialize(Archive& Ar) override;

		std::vector<uint8> CookData;
		physx::PxTriangleMesh* TriMesh;

		PhysicUserData UserData;
	};

	class BodySetup : public Serializable
	{
	public:

		BodySetup()
			: m_UseTriMesh(false)
		{
		}
		virtual ~BodySetup() {}

		virtual void Serialize( Archive& Ar ) override;

		AggregateGeom m_AggGeo;
		std::vector<TriMeshGeom> m_TriMeshes;

		bool m_UseTriMesh;

		// TODO: Add physic material

#if WITH_EDITOR
		void DrawDetailPanel();

		void DrawSphereShapes();
		void DrawBoxShapes();
		void DrawCapsuleShapes();
#endif

	private:

	};
}