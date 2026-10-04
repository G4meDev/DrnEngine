#include "DrnPCH.h"
#include "ConvexElem.h"

namespace Drn
{
	ConvexElem::ConvexElem( const std::vector<Vector>& InVertexData, const std::vector<std::vector<uint32>>& InPolyData, bool& bSuccess, bool bCompute )
		: ConvexElem()
	{
		PxConvexMeshDesc Desc;

		std::vector<PxVec3> Positions;
		for (const Vector& Pos : InVertexData)
		{
			Positions.push_back(Vector2P(Pos));
		}

		if (bCompute)
		{
			Desc.points = PxBoundedData(Positions.data(), sizeof(PxVec3), Positions.size());
			Desc.flags = PxConvexFlag::eCOMPUTE_CONVEX | PxConvexFlag::eCHECK_ZERO_AREA_TRIANGLES;
		}
		else
		{
			drn_check(false); // @TODO: fix. not working

			uint32 Index = 0;
			std::vector<uint32> Indices;
			std::vector<PxHullPolygon> Polys;
			for (const std::vector<uint32>& InPoly : InPolyData)
			{
				Polys.push_back({});
				PxHullPolygon& Poly = Polys.back();

				Poly.mNbVerts = InPoly.size();
				Poly.mIndexBase = Index;
				Index += Poly.mNbVerts;

				const Vector& P0 = InVertexData[InPoly[0]];
				const Vector& P1 = InVertexData[InPoly[1]];
				const Vector& P2 = InVertexData[InPoly[2]];

				Plane PolyPlane = Plane(P0, P1, P2);
				Poly.mPlane[0] = PolyPlane.X;
				Poly.mPlane[1] = PolyPlane.Y;
				Poly.mPlane[2] = PolyPlane.Z;
				Poly.mPlane[3] = PolyPlane.W;

				for (uint32 i : InPoly)
				{
					Indices.push_back(i);
				}
			}

			Desc.indices = PxBoundedData((void*)Indices.data(), sizeof(uint32), Indices.size());
			Desc.points = PxBoundedData(Positions.data(), sizeof(PxVec3), Positions.size());
			Desc.polygons = PxBoundedData(Polys.data(), sizeof(PxHullPolygon), Polys.size());
		}

		drn_check(Desc.isValid());

		PxDefaultMemoryOutputStream WriteStream;
		PxConvexMeshCookingResult::Enum result;
		physx::PxCookingParams CookParam = physx::PxCookingParams( physx::PxTolerancesScale());
		bSuccess = PxCookConvexMesh(CookParam, Desc, WriteStream, &result);

		if (!bSuccess)
		{
			drn_check(false);
			Reset();
			return;
		}

		CookData.resize(WriteStream.getSize());
		memcpy(CookData.data(), WriteStream.getData(), WriteStream.getSize());

		CreatePhysicMesh();
	}

	ConvexElem::ConvexElem( const ConvexElem& Other )
		: ConvexElem()
	{
		// @TODO: maybe add ref instead of nullifying convex mesh
		//ConvexMesh->acquireReference();
		//ConvexMesh = Other.ConvexMesh;
		CookData = Other.CookData;
		Type = Other.GetType();
	}

	void ConvexElem::Serialize( Archive& Ar )
	{
		if (Ar.IsLoading())
		{
			Ar.operator>> <uint32>(CookData);

			CreatePhysicMesh();
		}
		else
		{
			Ar.operator<< <uint32>(CookData);
		}
	}

	void ConvexElem::Reset()
	{
		CookData.clear();

		PX_RELEASE(ConvexMesh);
	}

	void ConvexElem::CreatePhysicMesh()
	{
		drn_check(!ConvexMesh);

		PxDefaultMemoryInputData ReadStream(CookData.data(), CookData.size());
		ConvexMesh = PhysicManager::Get()->GetPhysics()->createConvexMesh(ReadStream);

#if WITH_EDITOR
		const uint8* IndexBuffer = ConvexMesh->getIndexBuffer();
		const PxVec3* VertexBuffer = ConvexMesh->getVertices();
		const uint32 NumVertex = ConvexMesh->getNbVertices();

		VertexData.clear();
		IndexData.clear();
		for (int32 VertexIndex = 0; VertexIndex < NumVertex; VertexIndex++)
		{
			VertexData.push_back(P2Vector(VertexBuffer[VertexIndex]));
		}

		const uint32 PolyCount = ConvexMesh->getNbPolygons();
		for (uint32 PolyIndex = 0; PolyIndex < PolyCount; PolyIndex++)
		{
			PxHullPolygon Poly;
			ConvexMesh->getPolygonData(PolyIndex, Poly);

			const uint16 VertexCount = Poly.mNbVerts;
			const uint16 BaseVertex = Poly.mIndexBase;

			for (uint16 VertexIndex = 0; VertexIndex < VertexCount - 2; VertexIndex++)
			{
				IndexData.push_back(IndexBuffer[BaseVertex]);
				IndexData.push_back(IndexBuffer[BaseVertex+VertexIndex+1]);
				IndexData.push_back(IndexBuffer[BaseVertex+VertexIndex+2]);
			}
		}
#endif
	}

#if WITH_EDITOR
	void ConvexElem::DrawDebug( World* InWorld ) const
	{
		for (uint32 Index = 0; Index < IndexData.size(); Index+=3)
		{
			InWorld->DrawDebugLine(VertexData[IndexData[Index+0]], VertexData[IndexData[Index+1]], Color::White, 0, 0);
			InWorld->DrawDebugLine(VertexData[IndexData[Index+1]], VertexData[IndexData[Index+2]], Color::White, 0, 0);
			InWorld->DrawDebugLine(VertexData[IndexData[Index+2]], VertexData[IndexData[Index+0]], Color::White, 0, 0);
		}
	}
#endif

        }