#include "DrnPCH.h"
#include "RenderGeometeryHelper.h"

namespace Drn
{
	void RenderGeometeryHelper::CreateSpotlightStencilGeometery(ID3D12GraphicsCommandList2* CommandList, VertexBuffer*& VB, IndexBuffer*& IB)
	{
		const float Radius = 1.0f;
		const float Angle = XM_PIDIV4;

		std::vector<Vector> Points;
		std::vector<uint32> Indices;

		const float ZRadius = Radius * std::cos(Angle);
		const float ConeTan = std::tan(Angle);
		const float InvCosRadiansPerSide = 1.0f / std::cos(XM_PI / (float) SPOTLIGHT_STENCIL_SIDES);

		const float CapRadius = ZRadius * ConeTan;
		const int32 CapIndex = SPOTLIGHT_STENCIL_SIDES * SPOTLIGHT_STENCIL_SLICES;

		for (int32 SideIndex = 0; SideIndex < SPOTLIGHT_STENCIL_SIDES; SideIndex++)
		{
			for (int32 SliceIndex = 0; SliceIndex < SPOTLIGHT_STENCIL_SLICES; SliceIndex++)
			{
				const float CurrentAngle = SideIndex * 2 * XM_PI / (float) SPOTLIGHT_STENCIL_SIDES;
				const float DistanceAlongZ = ZRadius * SliceIndex / (float) (SPOTLIGHT_STENCIL_SLICES - 1);
				
				const float SliceRadius = DistanceAlongZ * ConeTan * InvCosRadiansPerSide;

				Points.emplace_back(SliceRadius * std::cos(CurrentAngle), SliceRadius * std::sin(CurrentAngle), DistanceAlongZ);
			}
		}

		// make cap
		for (int32 SideIndex = 0; SideIndex < SPOTLIGHT_STENCIL_SIDES; SideIndex++)
		{
			for (int32 SliceIndex = 0; SliceIndex < SPOTLIGHT_STENCIL_SLICES; SliceIndex++)
			{
				const float UnadjustedSliceRadius = CapRadius * SliceIndex / (float) (SPOTLIGHT_STENCIL_SLICES - 1);
				const float SliceRadius = UnadjustedSliceRadius * InvCosRadiansPerSide;

				const float CurrentAngle = SideIndex * 2 * XM_PI / (float) SPOTLIGHT_STENCIL_SIDES;
				const float DistanceAlongZ = std::sqrt(Radius * Radius - UnadjustedSliceRadius * UnadjustedSliceRadius);

				Points.emplace_back(SliceRadius * std::cos(CurrentAngle), SliceRadius * std::sin(CurrentAngle), DistanceAlongZ);
			}
		}

		for (int32 SideIndex = 0; SideIndex < SPOTLIGHT_STENCIL_SIDES; SideIndex++)
		{
			for (int32 SliceIndex = 0; SliceIndex < SPOTLIGHT_STENCIL_SLICES - 1; SliceIndex++)
			{
				int32 NextSide = (SideIndex + 1) % SPOTLIGHT_STENCIL_SIDES;

				int32 Point_1 = SideIndex * SPOTLIGHT_STENCIL_SLICES + SliceIndex; 
				int32 Point_2 = Point_1 + 1;

				int32 Point_3 = NextSide * SPOTLIGHT_STENCIL_SLICES + SliceIndex; 
				int32 Point_4 = Point_3 + 1;

				Indices.push_back(Point_1);
				Indices.push_back(Point_4);
				Indices.push_back(Point_2);

				if (SliceIndex != 0)
				{
					Indices.push_back(Point_1);
					Indices.push_back(Point_3);
					Indices.push_back(Point_4);
				}
			}
		}

		// make cap
		for (int32 SideIndex = 0; SideIndex < SPOTLIGHT_STENCIL_SIDES; SideIndex++)
		{
			for (int32 SliceIndex = 0; SliceIndex < SPOTLIGHT_STENCIL_SLICES - 1; SliceIndex++)
			{
				int32 NextSide = (SideIndex + 1) % SPOTLIGHT_STENCIL_SIDES;

				int32 Point_1 = SideIndex * SPOTLIGHT_STENCIL_SLICES + SliceIndex + CapIndex; 
				int32 Point_2 = Point_1 + 1;

				int32 Point_3 = NextSide * SPOTLIGHT_STENCIL_SLICES + SliceIndex + CapIndex;
				int32 Point_4 = Point_3 + 1;

				Indices.push_back(Point_1);
				Indices.push_back(Point_2);
				Indices.push_back(Point_4);

				if (SliceIndex != 0)
				{
					Indices.push_back(Point_1);
					Indices.push_back(Point_4);
					Indices.push_back(Point_3);
				}
			}
		}

		VB = VertexBuffer::Create( CommandList, Points.data(), Points.size(), sizeof( Vector ), "VB_SpotlightStencilGeometery");
		IB = IndexBuffer::Create( CommandList, Indices.data(), Indices.size(), Indices.size() * sizeof( uint32 ), DXGI_FORMAT_R32_UINT, "IB_SpotlightStencilGeometery");
	}



}