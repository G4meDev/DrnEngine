#pragma once

#include "ForwardTypes.h"
#include "AggregateGeom.h"

#include <PxPhysics.h>
#include <PxPhysicsAPI.h>

#define MAX_PHYSICS_MATERIAL_MASKS 8

namespace Drn
{
	enum class ECollisionTraceFlag : uint8
	{
		UseSimpleAndComplex,
		UseComplexAsSimple,
		UseSimpleAsComplex
	};

	enum class EPhysicsType : uint8
	{
		Default,
		Kinematic,
		Simulated
	};

	enum class EBodyCollisionResponse : uint8
	{
		Enabled,
		Disabled
	};

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

		BodySetup() {}
		virtual ~BodySetup() {}

		virtual void Serialize( Archive& Ar ) override;

		AggregateGeom m_AggGeo;
		std::vector<TriMeshGeom> m_TriMeshes;

		inline ECollisionTraceFlag GetTraceFlags() const { return CollisionTraceFlags; }
		inline bool TraceFlagSimpleAndComplex() const { return CollisionTraceFlags == ECollisionTraceFlag::UseSimpleAndComplex; }
		inline bool TraceFlagComplexAsSimple() const { return CollisionTraceFlags == ECollisionTraceFlag::UseComplexAsSimple; }
		inline bool TraceFlagSimpleAsComplex() const { return CollisionTraceFlags == ECollisionTraceFlag::UseSimpleAsComplex; }

		inline bool HasSimpleCollision() const { return m_AggGeo.GetElementCount() != 0; }
		inline bool HasComplexCollision() const { return !m_TriMeshes.empty(); }

		inline bool HasCollision() const
		{
			switch ( CollisionTraceFlags )
			{
			case ECollisionTraceFlag::UseSimpleAndComplex:		return HasSimpleCollision() || HasComplexCollision();
			case ECollisionTraceFlag::UseComplexAsSimple:		return HasComplexCollision();
			case ECollisionTraceFlag::UseSimpleAsComplex:		return HasSimpleCollision();
			default:											drn_check(false); return false;
			}
		}

// -----------------------------------------------------------------

		ECollisionTraceFlag CollisionTraceFlags;

		// not implemented
		//StringID BoneName;
		//
		//ECollisionTraceFlag CollisionTraceFlags;
		//EPhysicsType PhysicsType;
		//EBodyCollisionResponse CollisionResponse;
		//
		//bool bSupportUVFromHitResults;
		//bool bSupportMaterialMasks;
		//
		//AssetHandle<PhysicalMaterial> SimplePhysMaterial;
		//AssetHandle<PhysicalMaterial> ComplexPhysMaterial[MAX_PHYSICS_MATERIAL_MASKS];

		//FBodySetupUVInfo UVInfo;

// -----------------------------------------------------------------

#if WITH_EDITOR
		void DrawDetailPanel();

		bool DrawCollisionTraceFlag();

		void DrawSphereShapes();
		void DrawBoxShapes();
		void DrawCapsuleShapes();
#endif

	private:

	};
}