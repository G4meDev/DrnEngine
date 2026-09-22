#include "DrnPCH.h"
#include "NavMeshConvexHalfEdge.h"

#define NUM_VERTEX_PER_PLANE 3

namespace Drn
{

// bool NavMeshConvexHalfEdge::SetPlaneVertices( const std::vector<std::vector<uint32>>& InPlaneVertices, uint32
// InNumVertices )
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

	void NavMeshConvexHalfEdge::Clear()
	{
		Planes.clear();
		Vertices.clear();
		HalfEdges.clear();
	}

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

	bool NavMeshConvexHalfEdge::AddTriangleToEdge( uint32 EdgeIndex, const Vector& Position, uint32& NewVertexIndex )
	{
		drn_check(EdgeIndex < NumHalfEdges());

		if (GetTwinHalfEdge(EdgeIndex) == InvalidIndex)
		{
			NewVertexIndex = NumVertices();

			Planes.push_back({});
			Planes.back().FirstHalfEdgeIndex = NumHalfEdges();

			Vertices.push_back({});
			Vertices.back().Position = Position;
			Vertices.back().FirstHalfEdgeIndex = NumHalfEdges();

			const int32 PlaneIndex = NumPlanes() - 1;
			HalfEdges.push_back({});
			HalfEdges.back().PlaneIndex = PlaneIndex;
			HalfEdges.back().VertexIndex = GetHalfEdgeVertex(GetNextHalfEdge(EdgeIndex));
			HalfEdges.back().TwinHalfEdgeIndex = EdgeIndex;
			GetHalfEdge(EdgeIndex).TwinHalfEdgeIndex = NumHalfEdges() - 1;

			HalfEdges.push_back({});
			HalfEdges.back().PlaneIndex = PlaneIndex;
			HalfEdges.back().VertexIndex = GetHalfEdgeVertex(EdgeIndex);
			HalfEdges.back().TwinHalfEdgeIndex = InvalidIndex;

			HalfEdges.push_back({});
			HalfEdges.back().PlaneIndex = PlaneIndex;
			HalfEdges.back().VertexIndex = NewVertexIndex;
			HalfEdges.back().TwinHalfEdgeIndex = InvalidIndex;

			return true;
		}
		
		return false;
	}

	void NavMeshConvexHalfEdge::Rebuild()
	{
		// calculate center and plane
		for (uint32 PlaneIndex = 0; PlaneIndex < NumPlanes(); PlaneIndex++)
		{
			const Vector& P0 = GetVertex(GetPlaneVertex(PlaneIndex, 0)).Position;
			const Vector& P1 = GetVertex(GetPlaneVertex(PlaneIndex, 1)).Position;
			const Vector& P2 = GetVertex(GetPlaneVertex(PlaneIndex, 2)).Position;

			GetPlane(PlaneIndex).Center = (P0 + P1 + P2) / NUM_VERTEX_PER_PLANE;
			GetPlane(PlaneIndex).SurfacePlane = Plane(P0, P1, P2);
		}
	}

	void NavMeshConvexHalfEdge::DeletePlane( uint32 PlaneIndex )
	{
		drn_check(PlaneIndex < NumPlanes());

		const uint32 FirstEdgeToDelete = GetPlane(PlaneIndex).FirstHalfEdgeIndex;
		const uint32 NumDeletedEdges = NUM_VERTEX_PER_PLANE;

		std::vector<uint32> VerticesToDelete;

		VisitPlaneEdges( PlaneIndex, [&](uint32 HalfEdgeIndex, uint32 NextHalfEdgeIndex)
		{
			if (GetTwinHalfEdge(HalfEdgeIndex) != InvalidIndex)
			{
				GetHalfEdge(GetTwinHalfEdge(HalfEdgeIndex)).TwinHalfEdgeIndex = InvalidIndex;
			}

			const uint32 MaxPlanes = 20;
			uint32 FoundPlanes[MaxPlanes];
			//uint32 NumVertexFaces = FindVertexPlanes(GetHalfEdgeVertex(HalfEdgeIndex), FoundPlanes, MaxPlanes);
			uint32 NumVertexFaces = GetVertexPlanes(GetHalfEdgeVertex(HalfEdgeIndex)).size();

			if (NumVertexFaces < 2)
			{
				VerticesToDelete.push_back(GetHalfEdgeVertex(HalfEdgeIndex));
			}

			return true;
		});

		for (uint32 Index = 0; Index < NumVertices(); Index++)
		{
			if (GetVertex(Index).FirstHalfEdgeIndex > FirstEdgeToDelete)
			{
				GetVertex(Index).FirstHalfEdgeIndex -= NumDeletedEdges;
			}
		}

		for (uint32 Index = 0; Index < NumHalfEdges(); Index++)
		{
			if (GetHalfEdge(Index).PlaneIndex > PlaneIndex)
			{
				GetHalfEdge(Index).PlaneIndex--;
			}

			uint32 Dec = 0;
			for (uint32 VertexToDelete : VerticesToDelete)
			{
				if (GetHalfEdge(Index).VertexIndex > VertexToDelete)
				{
					Dec++;
				}
			}
			GetHalfEdge(Index).VertexIndex-=Dec;

			if (GetHalfEdge(Index).TwinHalfEdgeIndex != InvalidIndex)
			{
				if ((GetHalfEdge(Index).TwinHalfEdgeIndex >= FirstEdgeToDelete) && (GetHalfEdge(Index).TwinHalfEdgeIndex < FirstEdgeToDelete + NumDeletedEdges))
				{
					GetHalfEdge(Index).TwinHalfEdgeIndex = InvalidIndex;
				}
				else if (GetHalfEdge(Index).TwinHalfEdgeIndex >= FirstEdgeToDelete)
				{
					GetHalfEdge(Index).TwinHalfEdgeIndex -= NumDeletedEdges;
				}
			}
		}

		for (uint32 Index = 0; Index < NumPlanes(); Index++)
		{
			if (GetPlane(Index).FirstHalfEdgeIndex > FirstEdgeToDelete)
			{
				GetPlane(Index).FirstHalfEdgeIndex -= NumDeletedEdges;
			}
		}

		Planes.erase(Planes.begin() + PlaneIndex);
		HalfEdges.erase(HalfEdges.begin() + FirstEdgeToDelete, HalfEdges.begin() + FirstEdgeToDelete + NumDeletedEdges);

		std::sort( VerticesToDelete.begin(), VerticesToDelete.end(), []( uint32 A, uint32 B ) { return A > B;} );
		for (uint32 DeleteIndex : VerticesToDelete)
		{
			Vertices.erase(Vertices.begin() + DeleteIndex);
		}
	}

	void NavMeshConvexHalfEdge::DeletePlanes( std::vector<uint32> PlanesIndex )
	{
		std::sort(PlanesIndex.begin(), PlanesIndex.end(), [](uint32 A, uint32 B) { return A > B; });

		for (uint32 PlaneIndex : PlanesIndex)
		{
			DeletePlane(PlaneIndex);
		}
	}

	void NavMeshConvexHalfEdge::DeleteVertex( uint32 VertexIndex )
	{
		drn_check(VertexIndex < NumVertices());

		std::vector<uint32> Deletes = GetVertexPlanes(VertexIndex);
		DeletePlanes(Deletes);
	}

	void NavMeshConvexHalfEdge::DeleteEdge( uint32 EdgeIndex )
	{
		drn_check(EdgeIndex < NumHalfEdges());
		
		std::vector<uint32> Deletes;
		Deletes.push_back(GetHalfEdge(EdgeIndex).PlaneIndex);

		if (GetTwinHalfEdge(EdgeIndex) != InvalidIndex)
		{
			Deletes.push_back(GetHalfEdge(GetTwinHalfEdge(EdgeIndex)).PlaneIndex);
		}

		DeletePlanes(Deletes);
	}

	void NavMeshConvexHalfEdge::FillEdge( uint32 EdgeIndex )
	{
		drn_check(EdgeIndex < NumHalfEdges());

		if (GetTwinHalfEdge(EdgeIndex) != InvalidIndex)
		{
			return;
		}

		uint32 CenterVertex = GetHalfEdgeVertex(GetNextHalfEdge(EdgeIndex));
		std::vector<uint32> SharedPlanes = GetVertexPlanes(CenterVertex);
		std::vector<uint32> CandidateEdges;

		for (uint32 PlaneIndex = 0; PlaneIndex < NumPlanes(); PlaneIndex++)
		{
			if (PlaneIndex == GetHalfEdgePlane(EdgeIndex))
			{
				continue;
			}

			VisitPlaneEdges(PlaneIndex, [&](uint32 HalfEdgeIndex, uint32 NextHalfEdgeIndex)
			{
				if (GetHalfEdgeVertex(HalfEdgeIndex) == CenterVertex)
				{
					if (GetTwinHalfEdge(HalfEdgeIndex) == InvalidIndex)
					{
						CandidateEdges.push_back(HalfEdgeIndex);
					}

					return false;
				}

				return true;
			});
		}

		const Vector& Pt0 = GetVertex(GetHalfEdgeVertex(EdgeIndex)).Position;
		const Vector& Pt1 = GetVertex(CenterVertex).Position;
		const Vector BaseDir = (Pt1 - Pt0).GetSafeNormal();

		float MinDot = FLT_MAX;

		uint32 VerifiedEdge = InvalidIndex;
		for (uint32 Candidate : CandidateEdges)
		{
			const Vector& TargetPoint = GetVertex(GetHalfEdgeVertex(GetNextHalfEdge(Candidate))).Position;

			if (IsPointOutsideEdge(Pt0, Pt1, TargetPoint))
			{
				float Dot = BaseDir | (TargetPoint - CenterVertex).GetSafeNormal();

				if (Dot < MinDot)
				{
					MinDot = Dot;
					VerifiedEdge = Candidate;
				}
			}
		}

		if (VerifiedEdge == InvalidIndex)
		{
			return;
		}

		Planes.push_back({});
		Planes.back().FirstHalfEdgeIndex = NumHalfEdges();

		uint32 PlaneIndex = NumPlanes() - 1;
		HalfEdges.push_back({});
		HalfEdges.back().PlaneIndex = PlaneIndex;
		HalfEdges.back().VertexIndex = GetHalfEdgeVertex(GetNextHalfEdge(VerifiedEdge));
		HalfEdges.back().TwinHalfEdgeIndex = VerifiedEdge;
		GetHalfEdge(VerifiedEdge).TwinHalfEdgeIndex = HalfEdges.size() - 1;

		HalfEdges.push_back({});
		HalfEdges.back().PlaneIndex = PlaneIndex;
		HalfEdges.back().VertexIndex = CenterVertex;
		HalfEdges.back().TwinHalfEdgeIndex = EdgeIndex;
		GetHalfEdge(EdgeIndex).TwinHalfEdgeIndex = HalfEdges.size() - 1;

		HalfEdges.push_back({});
		HalfEdges.back().PlaneIndex = PlaneIndex;
		HalfEdges.back().VertexIndex = GetHalfEdgeVertex(EdgeIndex);

		uint32 Twin = InvalidIndex;
		for (uint32 PlaneIndex = 0; PlaneIndex < NumPlanes(); PlaneIndex++)
		{
			if (Twin != InvalidIndex)
			{
				break;
			}

			VisitPlaneEdges(PlaneIndex, [&](uint32 HalfEdgeIndex, uint32 NextHalfEdgeIndex)
			{
				const bool StartEquals = GetHalfEdgeVertex(HalfEdgeIndex) == GetHalfEdgeVertex(GetNextHalfEdge(VerifiedEdge));
				const bool EndEquals = GetHalfEdgeVertex(NextHalfEdgeIndex) == GetHalfEdgeVertex(EdgeIndex);

				if(StartEquals && EndEquals)
				{
					Twin = HalfEdgeIndex;
					return false;
				}

				return true;
			});
		}

		HalfEdges.back().TwinHalfEdgeIndex = Twin;
		if (Twin != InvalidIndex)
		{
			GetHalfEdge(Twin).TwinHalfEdgeIndex = HalfEdges.size() - 1;
		}

		if (VerifiedEdge != InvalidIndex)
		{
			const Vector& TargetPoint = GetVertex(GetHalfEdgeVertex(GetNextHalfEdge(VerifiedEdge))).Position;

			WorldManager::Get()->GetMainWorld()->DrawDebugSphere(TargetPoint, Quat::Identity, Color::Emerald, 0.5, 32, 0.0, 10.0f);
		}
	}

	void NavMeshConvexHalfEdge::MergePoints( uint32 Source, uint32 Target )
	{
		drn_check(Source < NumVertices());
		drn_check(Target < NumVertices());

		if (Source == Target)
		{
			return;
		}

		// @TODO: merging is for two separate islands. do not let merge on same plane and possibly making twins

		for (uint32 PlaneIndex = 0; PlaneIndex < NumPlanes(); PlaneIndex++)
		{
			VisitPlaneEdges( PlaneIndex, [&](uint32 HalfEdgeIndex, uint32 NextHalfEdgeIndex)
			{
				if (GetHalfEdgeVertex(HalfEdgeIndex) == Source)
				{
					GetHalfEdge(HalfEdgeIndex).VertexIndex = Target;
				}

				if (GetHalfEdgeVertex(HalfEdgeIndex) > Source)
				{
					GetHalfEdge(HalfEdgeIndex).VertexIndex--;
				}

				return true;
			});
		}

		Vertices.erase(Vertices.begin() + Source);
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

	uint32 NavMeshConvexHalfEdge::FindNearestPlane( const Vector& Point, Vector& NearestPosition, float Radius, float Height, bool& bOverPlane ) const
	{
		drn_check(Radius > 0.0f);
		drn_check(Height > 0.0f);

		uint32 Result = InvalidIndex;
		NearestPosition = Vector::ZeroVector;
		bOverPlane = false;

		for (uint32 PlaneIndex = 0; PlaneIndex < NumPlanes(); PlaneIndex++)
		{
			const Plane& SurfacePlane = GetPlane(PlaneIndex).SurfacePlane;
			const Vector& P0 = GetVertex(GetPlaneVertex(PlaneIndex, 0)).Position;
			const Vector& P1 = GetVertex(GetPlaneVertex(PlaneIndex, 1)).Position;
			const Vector& P2 = GetVertex(GetPlaneVertex(PlaneIndex, 2)).Position;

			Vector YProjected = SurfacePlane.RayIntersection(Point, Vector::UpVector);
			const Vector NearestOnPlane = Math::FindClosestPointOnTriangle(SurfacePlane, P0, P1, P2, YProjected);

			const float YDist = YProjected.Y - Point.Y;
			bool bInHeightRange = std::abs(YDist) <= Height;

			const float XZDist = (YProjected - NearestOnPlane).SizeSquaredXZ();
			const bool bOverlaps = XZDist <= KINDA_SMALL_NUMBER;
			const bool bInRadiusRange = XZDist <= (Radius * Radius);

			if (bInHeightRange)
			{
				if (bOverlaps)
				{
					bOverPlane = true;
					NearestPosition = NearestOnPlane;
					return PlaneIndex;
				}

				if (bInRadiusRange)
				{
					Result = PlaneIndex;
					NearestPosition = NearestOnPlane;
				}
			}
		}

		return Result;
	}

}