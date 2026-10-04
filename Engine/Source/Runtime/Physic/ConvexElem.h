#pragma once

#include "ForwardTypes.h"
#include "Runtime/Physic/ShapeElem.h"

namespace Drn
{
	class ConvexElem : public ShapeElem
	{
	public:

		ConvexElem()
			: ConvexMesh(nullptr)
			, ShapeElem(EAggCollisionShape::Convex)
		{}

		ConvexElem(const std::vector<Vector>& InVertexData, const std::vector<std::vector<uint32>>& InPolyData, bool& bSuccess, bool bCompute = true);

		ConvexElem(Archive& Ar)
			: ConvexElem()
		{
			Serialize(Ar);
		}

		ConvexElem(const ConvexElem& Other);

		virtual ~ConvexElem()
		{
			PX_RELEASE(ConvexMesh);
		}

		virtual void Serialize(Archive& Ar) override;

		inline virtual std::shared_ptr<PxGeometry> GetPxGeometery( const Vector& Scale ) override
		{
			//if (!ConvexMesh)
			//{
			//	CreatePhysicMesh();
			//}

			return std::shared_ptr<PxGeometry>(new PxConvexMeshGeometry(ConvexMesh, PxMeshScale(Vector2P(Scale))));
		}

		void Reset();

#if WITH_EDITOR
		void DrawDebug(World* InWorld) const;
#endif

	private:
		void CreatePhysicMesh();

		physx::PxConvexMesh* ConvexMesh;
		std::vector<uint8> CookData;

#if WITH_EDITOR
		std::vector<Vector> VertexData;
		std::vector<uint32> IndexData;
#endif
	};
}