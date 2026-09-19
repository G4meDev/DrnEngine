#include "DrnPCH.h"
#include "NavMeshConvexHalfEdge.h"

namespace Drn
{
	//bool NavMeshConvexHalfEdge::SetPlaneVertices( const std::vector<std::vector<uint32>>& InPlaneVertices, uint32 InNumVertices )
	//{
	//	int32 HalfEdgeCount = 0;
	//	for (int32 PlaneIndex = 0; PlaneIndex < InPlaneVertices.Num(); ++PlaneIndex)
	//	{
	//		HalfEdgeCount += InPlaneVertices[PlaneIndex].Num();
	//	}
	//
	//	if ((InPlaneVertices.Num() > MaxIndex) || (HalfEdgeCount > MaxIndex) || (InNumVertices > MaxIndex))
	//	{
	//		return false;
	//	}
	//
	//	Planes.SetNum(InPlaneVertices.Num());
	//	HalfEdges.SetNum(HalfEdgeCount);
	//	Vertices.SetNum(InNumVertices);
	//
	//	// Initialize the vertex list - it will be filled in as we build the edge list
	//	for (int32 VertexIndex = 0; VertexIndex < Vertices.Num(); ++VertexIndex)
	//	{
	//		GetVertex(VertexIndex).FirstHalfEdgeIndex = InvalidIndex;
	//	}
	//
	//	// Build the planes and edges. The edges for a plane are stored sequentially in the half-edge array.
	//	// On the first pass, the edges contain 2 vertex indices, rather than a vertex index and a twin edge index.
	//	// We fix this up on a second pass.
	//	int32 NextHalfEdgeIndex = 0;
	//	for (int32 PlaneIndex = 0; PlaneIndex < InPlaneVertices.Num(); ++PlaneIndex)
	//	{
	//		const TArray<int32>& PlaneVertices = InPlaneVertices[PlaneIndex];
	//
	//		GetPlane(PlaneIndex) =
	//		{
	//			(FIndex)NextHalfEdgeIndex,
	//			(FIndex)PlaneVertices.Num()
	//		};
	//
	//		for (int32 PlaneVertexIndex = 0; PlaneVertexIndex < PlaneVertices.Num(); ++PlaneVertexIndex)
	//		{
	//			// Add a new edge
	//			const int32 VertexIndex0 = PlaneVertices[PlaneVertexIndex];
	//			const int32 VertexIndex1 = PlaneVertices[(PlaneVertexIndex + 1) % PlaneVertices.Num()];
	//			GetHalfEdge(NextHalfEdgeIndex) =
	//			{
	//				(FIndex)PlaneIndex,
	//				(FIndex)VertexIndex0,
	//				(FIndex)VertexIndex1,	// Will get converted to a half-edge index later
	//			};
	//
	//			// If this is the first time Vertex0 has showed up, set its edge index
	//			if (Vertices[VertexIndex0].FirstHalfEdgeIndex == InvalidIndex)
	//			{
	//				Vertices[VertexIndex0].FirstHalfEdgeIndex = (FIndex)NextHalfEdgeIndex;
	//			}
	//
	//			++NextHalfEdgeIndex;
	//		}
	//	}
	//
	//	// Find the twin half edge for each edge
	//	// @todo(chaos): could use a map of vertex-index-pair to half edge to eliminate O(N^2) algorithm
	//	TArray<FIndex> TwinHalfEdgeIndices;
	//	TwinHalfEdgeIndices.SetNum(HalfEdges.Num());
	//	for (int32 HalfEdgeIndex = 0; HalfEdgeIndex < TwinHalfEdgeIndices.Num(); ++HalfEdgeIndex)
	//	{
	//		TwinHalfEdgeIndices[HalfEdgeIndex] = InvalidIndex;
	//	}
	//	for (int32 HalfEdgeIndex0 = 0; HalfEdgeIndex0 < HalfEdges.Num(); ++HalfEdgeIndex0)
	//	{
	//		const int32 VertexIndex0 = HalfEdges[HalfEdgeIndex0].VertexIndex;
	//		const int32 VertexIndex1 = HalfEdges[HalfEdgeIndex0].TwinHalfEdgeIndex;	// Actually a vertex index for now...
	//
	//		// Find the edge with the vertices the other way round
	//		for (int32 HalfEdgeIndex1 = 0; HalfEdgeIndex1 < HalfEdges.Num(); ++HalfEdgeIndex1)
	//		{
	//			if ((HalfEdges[HalfEdgeIndex1].VertexIndex == VertexIndex1) && (HalfEdges[HalfEdgeIndex1].TwinHalfEdgeIndex == VertexIndex0))
	//			{
	//				TwinHalfEdgeIndices[HalfEdgeIndex0] = (FIndex)HalfEdgeIndex1;
	//				break;
	//			}
	//		}
	//	}
	//
	//	// Set the twin edge indices
	//	for (int32 HalfEdgeIndex = 0; HalfEdgeIndex < HalfEdges.Num(); ++HalfEdgeIndex)
	//	{
	//		GetHalfEdge(HalfEdgeIndex).TwinHalfEdgeIndex = (FIndex)TwinHalfEdgeIndices[HalfEdgeIndex];
	//	}
	//
	//	return true;
	//}

	void NavMeshConvexHalfEdge::AddTriangle( std::vector<Vector>& Positions )
	{
		const uint32 VertexCount = Positions.size();
		drn_check(VertexCount == 3);

		const Vector Center = CalculatePointsCenter(Positions);
		const Vector Normal = CalculatePointsNormal(Positions);

		InverseTransformPoints(Positions, Center, Normal);
		SortPoints(Positions);
		TransformPoints(Positions, Center, Normal);

		Planes.push_back({});
		Planes.back().NumHalfEdges = VertexCount;
		Planes.back().FirstHalfEdgeIndex = HalfEdges.size();

		for (uint32 VertexIndex = 0; VertexIndex < VertexCount; VertexIndex++)
		{
			Vertices.push_back({});
			Vertices.back().Position = Positions[VertexIndex];
			Vertices.back().FirstHalfEdgeIndex = HalfEdges.size();

			HalfEdges.push_back({});
			HalfEdges.back().PlaneIndex = Planes.size() - 1;
			HalfEdges.back().VertexIndex = Vertices.size() - 1;
		}
	}

	Vector NavMeshConvexHalfEdge::CalculatePointsCenter( const std::vector<Vector>& Points )
	{
		Vector Result = Vector::ZeroVector;

		for (const Vector& Point : Points)
		{
			Result = Result + Point;
		}

		return Result / (float)Points.size();
	}

	Vector NavMeshConvexHalfEdge::CalculatePointsNormal( const std::vector<Vector>& Points )
	{
		drn_check(Points.size() >= 3);

		const Vector Edge1 = (Points[0] - Points[1]).GetSafeNormal();
		const Vector Edge2 = (Points[0] - Points[2]).GetSafeNormal();

		Vector Normal = (Edge1 ^ Edge2).GetSafeNormal();
		return Normal.Y > 0.0f ? Normal : Normal * -1.0f;
	}

	void NavMeshConvexHalfEdge::SortPoints( std::vector<Vector>& InOutPoints)
	{
		std::sort(InOutPoints.begin(), InOutPoints.end(), [](const Vector& A, const Vector& B)
		{
			float AngleA = Math::Atan2(A.X, A.Z);
			float AngleB = Math::Atan2(B.X, B.Z);

			return AngleA < AngleB;
		});
	}

	void NavMeshConvexHalfEdge::FlattenPoints( std::vector<Vector>& InOutPoints )
	{
		for (Vector& Point : InOutPoints)
		{
			Point.Y = 0;
		}
	}

	void NavMeshConvexHalfEdge::TransformPoints( std::vector<Vector>& Points, const Vector& Center, const Vector& Axis )
	{
		Quat Rotation = Quat::FromY(Axis);

		for (Vector& Point : Points)
		{
			Point = Rotation.RotateVector(Point) + Center;
		}
	}

	void NavMeshConvexHalfEdge::InverseTransformPoints( std::vector<Vector>& Points, const Vector& Center, const Vector& Axis )
	{
		Quat Rotation = Quat::FromY(Axis).Inverse();

		for (Vector& Point : Points)
		{
			Point = Rotation.RotateVector(Point - Center);
		}
	}

	float EdgePointValue(const Vector& A, const Vector& B, const Vector& Point)
	{
		return (B.X - A.X) * (Point.Z - A.Z) - (B.Z - A.Z) * (Point.X - A.X);
	}

	bool NavMeshConvexHalfEdge::IsPointInsideEdge( const Vector& A, const Vector& B, const Vector& Point )
	{
		return EdgePointValue(A, B, Point) < -SMALL_NUMBER;
	}

	bool NavMeshConvexHalfEdge::IsPointOutsideEdge( const Vector& A, const Vector& B, const Vector& Point )
	{
		return EdgePointValue(A, B, Point) > SMALL_NUMBER;
	}

	void NavMeshConvexHalfEdge::ClampPointToEdge( const Vector& A, const Vector& B, Vector& Point )
	{
		const Vector AB = B - A;
		const Vector AC = Point - A;

		const float Scale = (AB | AC) / (AB | AB);
		const Vector AD = AB * Math::Clamp(Scale, 0.02f, 0.98f);
		Point = A + AD;
	}

	void NavMeshConvexHalfEdge::GetIndexedFaces( std::vector<Vector>& Positions, std::vector<uint32>& Indices, bool bShareVertecies )
	{
		drn_check(!bShareVertecies);

		if (!bShareVertecies)
		{
			uint32 VertexCount = 0;
			for (int32 PlaneIndex = 0; PlaneIndex < NumPlanes(); PlaneIndex++)
			{
				VertexCount += NumPlaneVertices(PlaneIndex);
			}

			Positions.clear();
			Positions.reserve(VertexCount);
			for (int32 PlaneIndex = 0; PlaneIndex < NumPlanes(); PlaneIndex++)
			{
				for (uint32 VertexIndex = 0; VertexIndex < NumPlaneVertices(PlaneIndex); VertexIndex++)
				{
					Positions.push_back(Vertices[GetPlaneVertex(PlaneIndex, VertexIndex)].Position);
				}
			}

			Indices.clear();
			Indices.reserve(VertexCount);
			for (uint32 VertexIndex = 0; VertexIndex < VertexCount; VertexIndex++)
			{
				Indices.push_back(VertexIndex);
			}
		}
	}

	void NavMeshConvexHalfEdge::GetVeteciesPosition( std::vector<Vector>& Positions )
	{
		Positions.clear();
		Positions.reserve(NumVertices());

		for (uint32 VertexIndex = 0; VertexIndex < NumVertices(); VertexIndex++)
		{
			Positions.push_back(GetVertex(VertexIndex).Position);
		}
	}

	std::vector<Vector> NavMeshConvexHalfEdge::GetPlanePointsPosition( int32 PlaneIndex ) const
	{
		drn_check(PlaneIndex < NumPlanes());

		std::vector<Vector> Result;
		for (int32 VertexIndex = 0; VertexIndex < NumPlaneVertices(PlaneIndex); VertexIndex++)
		{
			Result.push_back(Vertices[GetPlaneVertex(PlaneIndex, VertexIndex)].Position);
		}

		return Result;
	}

	Transform NavMeshConvexHalfEdge::GetPlaneTransform( int32 PlaneIndex ) const
	{
		drn_check(PlaneIndex < NumPlanes());

		std::vector<Vector> Points = GetPlanePointsPosition(PlaneIndex);

		const Vector Center = CalculatePointsCenter(Points);
		const Vector Normal = CalculatePointsNormal(Points);

		return Transform(Center, Quat::FromY(Normal));
	}

	bool NavMeshConvexHalfEdge::SetPlaneTransform( int32 PlaneIndex, const Transform& InTransform )
	{
		drn_check(PlaneIndex < NumPlanes());

		const Transform CurrentTransform = GetPlaneTransform(PlaneIndex);
		bool bChanged = !CurrentTransform.NearlyEquals(InTransform);

		if (bChanged)
		{
			for (int32 VertexIndex = 0; VertexIndex < NumPlaneVertices(PlaneIndex); VertexIndex++)
			{
				VertexData& Vertex = Vertices[GetPlaneVertex(PlaneIndex, VertexIndex)]; 

				Vertex.Position = CurrentTransform.InverseTransformPosition(Vertex.Position);
				Vertex.Position = InTransform.TransformPosition(Vertex.Position);
			}
		}

		return bChanged;
	}

	Transform NavMeshConvexHalfEdge::GetVertexTransform( int32 VertexIndex ) const
	{
		drn_check(VertexIndex < NumVertices());

		return Transform(GetVertex(VertexIndex).Position);
	}

	bool NavMeshConvexHalfEdge::SetVertexTransform( int32 VertexIndex, const Transform& InTransform )
	{
		drn_check(VertexIndex < NumVertices());

		const Transform CurrentTransform = GetVertexTransform(VertexIndex);
		bool bChanged = !CurrentTransform.NearlyEquals(InTransform);

		if (bChanged)
		{
			VertexData& Vertex = GetVertex(VertexIndex);

			Vertex.Position = CurrentTransform.InverseTransformPosition(Vertex.Position);
			Vertex.Position = InTransform.TransformPosition(Vertex.Position);
		}

		return bChanged;
	}

	Transform NavMeshConvexHalfEdge::GetEdgeTransform( int32 EdgeIndex ) const
	{
		drn_check(EdgeIndex < NumHalfEdges());

		const Vector& EdgePt0 = GetVertex(GetHalfEdgeVertex(EdgeIndex)).Position;
		const Vector& EdgePt1 = GetVertex(GetHalfEdgeVertex(GetNextHalfEdge(EdgeIndex))).Position;
		const Vector& Dir = (EdgePt1 - EdgePt0).GetSafeNormal();
		const Vector Center = (EdgePt0 + EdgePt1) / 2;

		return Transform(Center, Quat::FromZ(Dir));
	}

	bool NavMeshConvexHalfEdge::SetEdgeTransform( int32 EdgeIndex, const Transform& InTransform )
	{
		drn_check(EdgeIndex < NumHalfEdges());

		const Transform CurrentTransform = GetEdgeTransform(EdgeIndex);
		bool bChanged = !CurrentTransform.NearlyEquals(InTransform);

		if (bChanged)
		{
			Vector& EdgePt0 = GetVertex(GetHalfEdgeVertex(EdgeIndex)).Position;
			Vector& EdgePt1 = GetVertex(GetHalfEdgeVertex(GetNextHalfEdge(EdgeIndex))).Position;

			EdgePt0 = CurrentTransform.InverseTransformPosition(EdgePt0);
			EdgePt0 = InTransform.TransformPosition(EdgePt0);

			EdgePt1 = CurrentTransform.InverseTransformPosition(EdgePt1);
			EdgePt1 = InTransform.TransformPosition(EdgePt1);
		}

		return bChanged;
	}

}  // namespace Drn