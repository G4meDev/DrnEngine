#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class NavMeshConvexHalfEdge
	{
	public:
		static const uint32 InvalidIndex = UINT32_MAX;
		static const uint32 MaxIndex = UINT32_MAX - 1;

		struct PlaneData
		{
			PlaneData()
				: FirstHalfEdgeIndex(InvalidIndex)
				, NumHalfEdges(0)
			{}

			//Vector Center;

			uint32 FirstHalfEdgeIndex;
			uint32 NumHalfEdges;
		};

		struct HalfEdgeData
		{
			HalfEdgeData()
				: PlaneIndex(InvalidIndex)
				, VertexIndex(InvalidIndex)
				, TwinHalfEdgeIndex(InvalidIndex)
			{}

			uint32 PlaneIndex;
			uint32 VertexIndex;
			uint32 TwinHalfEdgeIndex;
		};

		struct VertexData
		{
			Vector Position;

			uint32 FirstHalfEdgeIndex;

			VertexData()
				: FirstHalfEdgeIndex(InvalidIndex)
			{}
		};

		void AddTriangle(std::vector<Vector>& Positions);

		static Vector CalculatePointsCenter(const std::vector<Vector>& Points);
		static Vector CalculatePointsNormal(const std::vector<Vector>& Points);
		static void SortPoints(std::vector<Vector>& InOutPoints);
		static void FlattenPoints(std::vector<Vector>& InOutPoints);
		static void TransformPoints(std::vector<Vector>& Points, const Vector& Center, const Vector& Axis);
		static void InverseTransformPoints(std::vector<Vector>& Points, const Vector& Center, const Vector& Axis);
		static bool IsPointInsideEdge(const Vector& A, const Vector& B, const Vector& Point);
		static bool IsPointOutsideEdge(const Vector& A, const Vector& B, const Vector& Point);
		static void ClampPointToEdge(const Vector& A, const Vector& B, Vector& Point);

		void GetIndexedFaces(std::vector<Vector>& Positions, std::vector<uint32>& Indices, bool bShareVertecies = false);
		void GetVeteciesPosition(std::vector<Vector>& Positions);
		std::vector<Vector> GetPlanePointsPosition(int32 PlaneIndex) const;

		Transform GetPlaneTransform(int32 PlaneIndex) const;
		bool SetPlaneTransform(int32 PlaneIndex, const Transform& InTransform);

		Transform GetVertexTransform(int32 VertexIndex) const;
		bool SetVertexTransform(int32 VertexIndex, const Transform& InTransform);

		Transform GetEdgeTransform(int32 EdgeIndex) const;
		bool SetEdgeTransform(int32 EdgeIndex, const Transform& InTransform);

		//static NavMeshConvexHalfEdge MakePlaneVertices(const std::vector<std::vector<uint32>>& InPlaneVertices, uint32 InNumVertices)
		//{
		//	NavMeshConvexHalfEdge StructureData;
		//	StructureData.SetPlaneVertices(InPlaneVertices, InNumVertices);
		//	return StructureData;
		//}

		static bool CanMake(const std::vector<std::vector<uint32>>& InPlaneVertices, uint32 InNumVertices)
		{
			uint32 HalfEdgeCount = 0;
			for (uint32 PlaneIndex = 0; PlaneIndex < InPlaneVertices.size(); ++PlaneIndex)
			{
				HalfEdgeCount += InPlaneVertices[PlaneIndex].size();
			}
		
			return ((HalfEdgeCount <= MaxIndex) && (InPlaneVertices.size() <= MaxIndex) && (InNumVertices <= MaxIndex));
		}

		inline bool IsValid() const { return Planes.size() > 0; }
		inline uint32 NumPlanes() const { return Planes.size(); }
		inline uint32 NumHalfEdges() const { return HalfEdges.size(); }
		inline uint32 NumVertices() const { return Vertices.size(); }

		inline PlaneData& GetPlane(uint32 PlaneIndex) { return Planes[PlaneIndex]; }
		inline const PlaneData& GetPlane(uint32 PlaneIndex) const { return Planes[PlaneIndex]; }
		inline HalfEdgeData& GetHalfEdge(uint32 HalfEdgeIndex) { return HalfEdges[HalfEdgeIndex]; }
		inline const HalfEdgeData& GetHalfEdge(uint32 HalfEdgeIndex) const { return HalfEdges[HalfEdgeIndex]; }
		inline VertexData& GetVertex(uint32 VertexIndex) { return Vertices[VertexIndex]; }
		inline const VertexData& GetVertex(uint32 VertexIndex) const { return Vertices[VertexIndex]; }

		uint32 NumPlaneHalfEdges(uint32 PlaneIndex) const
		{
			return GetPlane(PlaneIndex).NumHalfEdges;
		}

		uint32 GetPlaneHalfEdge(uint32 PlaneIndex, uint32 PlaneEdgeIndex) const
		{
			drn_check(PlaneEdgeIndex >= 0);
			drn_check(PlaneEdgeIndex < NumPlaneHalfEdges(PlaneIndex));
			return GetPlane(PlaneIndex).FirstHalfEdgeIndex + PlaneEdgeIndex;
		}

		uint32 NumPlaneVertices(uint32 PlaneIndex) const
		{
			return GetPlane(PlaneIndex).NumHalfEdges;
		}

		uint32 GetPlaneVertex(uint32 PlaneIndex, uint32 PlaneVertexIndex) const
		{
			const uint32 HalfEdgeIndex = GetPlaneHalfEdge(PlaneIndex, PlaneVertexIndex);
			return GetHalfEdge(HalfEdgeIndex).VertexIndex;
		}

		uint32 GetHalfEdgePlane(uint32 HalfEdgeIndex) const
		{
			return GetHalfEdge(HalfEdgeIndex).PlaneIndex;
		}

		uint32 GetHalfEdgeVertex(uint32 HalfEdgeIndex) const
		{
			return GetHalfEdge(HalfEdgeIndex).VertexIndex;
		}

		uint32 GetTwinHalfEdge(uint32 HalfEdgeIndex) const
		{
			return GetHalfEdge(HalfEdgeIndex).TwinHalfEdgeIndex;
		}

		uint32 GetPrevHalfEdge(uint32 HalfEdgeIndex) const
		{
			const uint32 PlaneIndex = GetHalfEdge(HalfEdgeIndex).PlaneIndex;
			const uint32 PlaneHalfEdgeIndex = HalfEdgeIndex - GetPlane(PlaneIndex).FirstHalfEdgeIndex;
			return GetPrevPlaneHalfEdge(PlaneIndex, PlaneHalfEdgeIndex);
		}

		uint32 GetNextHalfEdge(uint32 HalfEdgeIndex) const
		{
			const uint32 PlaneIndex = GetHalfEdge(HalfEdgeIndex).PlaneIndex;
			const uint32 PlaneHalfEdgeIndex = HalfEdgeIndex - GetPlane(PlaneIndex).FirstHalfEdgeIndex;
			return GetNextPlaneHalfEdge(PlaneIndex, PlaneHalfEdgeIndex);
		}

		uint32 GetVertexFirstHalfEdge(uint32 VertexIndex) const
		{
			return GetVertex(VertexIndex).FirstHalfEdgeIndex;
		}

		//void VisitPlaneEdges(uint32 PlaneIndex, const TFunction<bool(uint32 HalfEdgeIndex, uint32 NextHalfEdgeIndex)>& Visitor) const
		//{
		//	const uint32 FirstHalfEdgeIndex = GetPlane(PlaneIndex).FirstHalfEdgeIndex;
		//	uint32 HalfEdgeIndex0 = FirstHalfEdgeIndex;
		//	if (HalfEdgeIndex0 != InvalidIndex)
		//	{
		//		bool bContinue = true;
		//		do
		//		{
		//			const uint32 HalfEdgeIndex1 = GetNextHalfEdge(HalfEdgeIndex0);
		//			if (HalfEdgeIndex1 != InvalidIndex)
		//			{
		//				bContinue = Visitor(HalfEdgeIndex0, HalfEdgeIndex1);
		//			}
		//			HalfEdgeIndex0 = HalfEdgeIndex1;
		//		} while (bContinue && (HalfEdgeIndex0 != FirstHalfEdgeIndex) && (HalfEdgeIndex0 != InvalidIndex));
		//	}
		//}
		//
		//void VisitVertexHalfEdges(uint32 VertexIndex, const TFunction<bool(uint32 HalfEdgeIndex)>& Visitor) const
		//{
		//	const uint32 FirstHalfEdgeIndex = GetVertex(VertexIndex).FirstHalfEdgeIndex;
		//	uint32 HalfEdgeIndex = FirstHalfEdgeIndex;
		//	if (HalfEdgeIndex != InvalidIndex)
		//	{
		//		bool bContinue = true;
		//		do
		//		{
		//			bContinue = Visitor(HalfEdgeIndex);
		//			const uint32 TwinHalfEdgeIndex = GetTwinHalfEdge(HalfEdgeIndex);
		//			if (TwinHalfEdgeIndex == InvalidIndex)
		//			{
		//				break;
		//			}
		//			HalfEdgeIndex = GetNextHalfEdge(TwinHalfEdgeIndex);
		//		} while (bContinue && (HalfEdgeIndex != FirstHalfEdgeIndex) && (HalfEdgeIndex != InvalidIndex));
		//	}
		//}
		//
		//uint32 FindVertexPlanes(uint32 VertexIndex, uint32* PlaneIndices, uint32 MaxVertexPlanes) const
		//{
		//	uint32 NumPlanesFound = 0;
		//
		//	if (MaxVertexPlanes > 0)
		//	{
		//		VisitVertexHalfEdges(VertexIndex,
		//			[this, PlaneIndices, MaxVertexPlanes, &NumPlanesFound](uint32 HalfEdgeIndex)
		//			{
		//				PlaneIndices[NumPlanesFound++] = GetHalfEdgePlane(HalfEdgeIndex);
		//				return (NumPlanesFound < MaxVertexPlanes);
		//			});
		//	}
		//
		//	return NumPlanesFound;
		//}

		//bool SetPlaneVertices(const std::vector<std::vector<uint32>>& InPlaneVertices, uint32 InNumVertices);

		uint32 GetPrevPlaneHalfEdge(uint32 PlaneIndex, uint32 PlaneHalfEdgeIndex) const
		{
			drn_check(PlaneHalfEdgeIndex >= 0);
			drn_check(PlaneHalfEdgeIndex < NumPlaneHalfEdges(PlaneIndex));
			const uint32 PlaneHalfEdgeCount = NumPlaneHalfEdges(PlaneIndex);
			const uint32 PrevPlaneHalfEdgeIndex = (PlaneHalfEdgeIndex + PlaneHalfEdgeCount - 1) % PlaneHalfEdgeCount;
			return GetPlaneHalfEdge(PlaneIndex, PrevPlaneHalfEdgeIndex);
		}

		uint32 GetNextPlaneHalfEdge(uint32 PlaneIndex, uint32 PlaneHalfEdgeIndex) const
		{
			drn_check(PlaneHalfEdgeIndex >= 0);
			drn_check(PlaneHalfEdgeIndex < NumPlaneHalfEdges(PlaneIndex));
			const uint32 PlaneHalfEdgeCount = NumPlaneHalfEdges(PlaneIndex);
			const uint32 NextPlaneHalfEdgeIndex = (PlaneHalfEdgeIndex + 1) % PlaneHalfEdgeCount;
			return GetPlaneHalfEdge(PlaneIndex, NextPlaneHalfEdgeIndex);
		}


		std::vector<PlaneData> Planes;
		std::vector<HalfEdgeData> HalfEdges;
		std::vector<VertexData> Vertices;
	};
}