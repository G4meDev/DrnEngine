#pragma once

#include "ForwardTypes.h"
#include "AggregateGeom.h"

#include <PxConfig.h>
#include <PxPhysics.h>
#include <PxPhysicsAPI.h>

namespace Drn
{
	class PxMemoryStream : public PxOutputStream, public PxInputStream
	{
	public:
		PxMemoryStream() 
		{}

		virtual ~PxMemoryStream()
		{}

		uint32 read( void* dest, uint32 count ) override
		{
			memcpy(dest, m_Buffer.data(), count);
			return count;
		}
		uint32 write( const void* src, uint32 count ) override
		{
			m_Buffer.resize(count);
			memcpy(m_Buffer.data(), src, count);
			return count;
		}

		std::vector<uint8> m_Buffer;
	};

	struct TriMeshGeom : public Serializable
	{
		TriMeshGeom()
			: TriMesh(nullptr)
		{};

		~TriMeshGeom() { ReleaseTriMesh(); }
		void ReleaseTriMesh() { PX_RELEASE(TriMesh); }

		virtual void Serialize(Archive& Ar) override;

		PxMemoryStream CookData;
		physx::PxTriangleMesh* TriMesh;
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