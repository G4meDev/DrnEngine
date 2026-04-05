
#include "Common.hlsl"

#define THREADGROUP_SIZE 4
#define LIGHT_GRID_NUM_CULLED_PRIMITIVE_TYPES 2;

struct Resources
{
    uint ViewIndex;
    uint LightGridIndex;
};

ConstantBuffer<Resources> BindlessResources : register(b0);

float ComputeCellNearViewDepthFromZSlice(LightGridData LightGrid, uint ZSlice)
{
    float SliceDepth = (exp2(ZSlice / LightGrid.LightGridZParams.z) - LightGrid.LightGridZParams.y) / LightGrid.LightGridZParams.x;
    
    if (ZSlice == (uint) LightGrid.CulledGridSize.z)
    {
        SliceDepth = 2000000.0f;
    }
    
    if (ZSlice == 0)
    {
        SliceDepth = 0.0f;
    }
    
    return SliceDepth;
}

float ComputeSquaredDistanceFromBoxToPoint(float3 BoxCenter, float3 BoxExtent, float3 InPoint)
{
    float3 AxisDistances = max(abs(InPoint - BoxCenter) - BoxExtent, 0);
    return dot(AxisDistances, AxisDistances);
}

void ComputeCellViewAABB(ViewBuffer View, LightGridData LightGrid, uint3 GridCoordinate, out float3 ViewTileMin, out float3 ViewTileMax)
{
    const float2 InvCulledGridSizeF = (1u << LightGrid.LightGridPixelSizeShift) * View.InvSize;
    const float2 TileSize = float2(2.0f, -2.0f) * InvCulledGridSizeF.xy;
    const float2 UnitPlaneMin = float2(-1.0f, 1.0f);

    float2 UnitPlaneTileMin = GridCoordinate.xy * TileSize + UnitPlaneMin;
    float2 UnitPlaneTileMax = (GridCoordinate.xy + 1) * TileSize + UnitPlaneMin;

    float MinTileZ = ComputeCellNearViewDepthFromZSlice(LightGrid, GridCoordinate.z);
    float MaxTileZ = ComputeCellNearViewDepthFromZSlice(LightGrid, GridCoordinate.z + 1);

    float MinTileDeviceZ = ConvertToDeviceZ(MinTileZ, View);
    //float4 p1 = mul(View.ViewToProjection, float4(0, 0, MinTileZ, 1));
    //float MinTileDeviceZ = p1.z / p1.w;
    float4 MinDepthCorner0 = mul(View.ProjectionToView, float4(UnitPlaneTileMin.x, UnitPlaneTileMin.y, MinTileDeviceZ, 1));
    float4 MinDepthCorner1 = mul(View.ProjectionToView, float4(UnitPlaneTileMax.x, UnitPlaneTileMax.y, MinTileDeviceZ, 1));
    float4 MinDepthCorner2 = mul(View.ProjectionToView, float4(UnitPlaneTileMin.x, UnitPlaneTileMax.y, MinTileDeviceZ, 1));
    float4 MinDepthCorner3 = mul(View.ProjectionToView, float4(UnitPlaneTileMax.x, UnitPlaneTileMin.y, MinTileDeviceZ, 1));

    float MaxTileDeviceZ = ConvertToDeviceZ(MaxTileZ, View);
    //float4 p2 = mul(View.ViewToProjection, float4(0, 0, MaxTileZ, 1));
    //float MaxTileDeviceZ = p2.z / p2.w;
    float4 MaxDepthCorner0 = mul(View.ProjectionToView, float4(UnitPlaneTileMin.x, UnitPlaneTileMin.y, MaxTileDeviceZ, 1));
    float4 MaxDepthCorner1 = mul(View.ProjectionToView, float4(UnitPlaneTileMax.x, UnitPlaneTileMax.y, MaxTileDeviceZ, 1));
    float4 MaxDepthCorner2 = mul(View.ProjectionToView, float4(UnitPlaneTileMin.x, UnitPlaneTileMax.y, MaxTileDeviceZ, 1));
    float4 MaxDepthCorner3 = mul(View.ProjectionToView, float4(UnitPlaneTileMax.x, UnitPlaneTileMin.y, MaxTileDeviceZ, 1));

    float2 ViewMinDepthCorner0 = MinDepthCorner0.xy / MinDepthCorner0.w;
    float2 ViewMinDepthCorner1 = MinDepthCorner1.xy / MinDepthCorner1.w;
    float2 ViewMinDepthCorner2 = MinDepthCorner2.xy / MinDepthCorner2.w;
    float2 ViewMinDepthCorner3 = MinDepthCorner3.xy / MinDepthCorner3.w;
    float2 ViewMaxDepthCorner0 = MaxDepthCorner0.xy / MaxDepthCorner0.w;
    float2 ViewMaxDepthCorner1 = MaxDepthCorner1.xy / MaxDepthCorner1.w;
    float2 ViewMaxDepthCorner2 = MaxDepthCorner2.xy / MaxDepthCorner2.w;
    float2 ViewMaxDepthCorner3 = MaxDepthCorner3.xy / MaxDepthCorner3.w;

    ViewTileMin.xy = min(ViewMinDepthCorner0, ViewMinDepthCorner1);
    ViewTileMin.xy = min(ViewTileMin.xy, ViewMinDepthCorner2);
    ViewTileMin.xy = min(ViewTileMin.xy, ViewMinDepthCorner3);
    ViewTileMin.xy = min(ViewTileMin.xy, ViewMaxDepthCorner0);
    ViewTileMin.xy = min(ViewTileMin.xy, ViewMaxDepthCorner1);
    ViewTileMin.xy = min(ViewTileMin.xy, ViewMaxDepthCorner2);
    ViewTileMin.xy = min(ViewTileMin.xy, ViewMaxDepthCorner3);

    ViewTileMax.xy = max(ViewMinDepthCorner0, ViewMinDepthCorner1);
    ViewTileMax.xy = max(ViewTileMax.xy, ViewMinDepthCorner2);
    ViewTileMax.xy = max(ViewTileMax.xy, ViewMinDepthCorner3);
    ViewTileMax.xy = max(ViewTileMax.xy, ViewMaxDepthCorner0);
    ViewTileMax.xy = max(ViewTileMax.xy, ViewMaxDepthCorner1);
    ViewTileMax.xy = max(ViewTileMax.xy, ViewMaxDepthCorner2);
    ViewTileMax.xy = max(ViewTileMax.xy, ViewMaxDepthCorner3);

    ViewTileMin.z = MinTileZ;
    ViewTileMax.z = MaxTileZ;
}

bool AabbOutsidePlane(float3 center, float3 extents, float4 plane)
{
    float dist = dot(float4(center, 1.0), plane);
    float radius = dot(extents, abs(plane.xyz));

    return dist > radius;
}

bool IsAabbOutsideInfiniteAcuteConeApprox(float3 ConeVertex, float3 ConeAxis, float TanConeAngle, float3 AabbCentre, float3 AabbExt)
{
    float3 D = AabbCentre - ConeVertex;

    float3 M = -normalize(cross(cross(D, ConeAxis), ConeAxis));
    float3 N = -TanConeAngle * ConeAxis + M;
    float4 Plane = float4(N, 0.0);

    return AabbOutsidePlane(D, AabbExt, Plane);
}

[numthreads(THREADGROUP_SIZE, THREADGROUP_SIZE, THREADGROUP_SIZE)]
void Main_CS(uint3 GroupId : SV_GroupID, uint3 DispatchThreadId : SV_DispatchThreadID, uint3 GroupThreadId : SV_GroupThreadID)
{
    ConstantBuffer<ViewBuffer> View = ResourceDescriptorHeap[BindlessResources.ViewIndex];
    ConstantBuffer<LightGridData> LightGrid = ResourceDescriptorHeap[BindlessResources.LightGridIndex];
    
    uint3 GridCoordinate = DispatchThreadId;
    
    if (all(GridCoordinate < (uint3) LightGrid.CulledGridSize))
    {
        uint GridIndex = (GridCoordinate.z * LightGrid.CulledGridSize.y + GridCoordinate.y) * LightGrid.CulledGridSize.x + GridCoordinate.x;

#define DISABLE_CULLS 0
        
        float3 ViewTileMin;
        float3 ViewTileMax;
        ComputeCellViewAABB(View, LightGrid, GridCoordinate, ViewTileMin, ViewTileMax);

        float3 ViewTileCenter = .5f * (ViewTileMin + ViewTileMax);
        float3 ViewTileExtent = ViewTileMax - ViewTileCenter;
        float3 WorldTileCenter = mul(View.ViewToWorld, float4(ViewTileCenter, 1)).xyz;
        float4 WorldTileBoundingSphere = float4(WorldTileCenter, length(ViewTileExtent));
        
        //uint NumAvailableLinks = LightGrid.NumGridCells * LightGrid.MaxCulledLightsPerCell * LIGHT_GRID_NUM_CULLED_PRIMITIVE_TYPES;
        uint NumAvailableLinks = LightGrid.NumGridCells * LightGrid.MaxCulledLightsPerCell;

        ConstantBuffer<LightGridPackedLocalLightData> PackedLocalLights = ResourceDescriptorHeap[LightGrid.LocalLightBufferIndex];
        ConstantBuffer<LightGridViewSpacePositionAndRadius> ViewSpacePositionAndRadiusBuffer = ResourceDescriptorHeap[LightGrid.ViewSpacePositionAndRadiusIndex];
        ConstantBuffer<LightGridViewSpaceDirAndPreprocAngle> ViewSpaceDirAndPreprocAngleBuffer = ResourceDescriptorHeap[LightGrid.ViewSpaceDirAndPreprocAngleIndex];
        
        RWBuffer<uint> RWNumCulledLightsGrid = ResourceDescriptorHeap[LightGrid.RWNumCulledLightsGridIndex];
        RWBuffer<uint> RWCulledLightsGrid = ResourceDescriptorHeap[LightGrid.RWCulledLightsGridIndex];
        
        RWBuffer<uint> RWNextCulledLightLink = ResourceDescriptorHeap[LightGrid.RWNextCulledLightLinkIndex];
        RWBuffer<uint> RWCulledLightLink = ResourceDescriptorHeap[LightGrid.RWCulledLightLinkIndex];
        RWBuffer<uint> RWStartGridOffset = ResourceDescriptorHeap[LightGrid.RWStartGridOffsetIndex];
        
        [loop]
        for (uint LocalLightIndex = 0; LocalLightIndex < LightGrid.NumCulledLights; LocalLightIndex++)
        {
            LightGridLocalLightData LocalLightData = GetLocalLightData(PackedLocalLights, LocalLightIndex);
            
            float4 LightPositionAndInvRadius = LocalLightData.LightPositionAndInvRadius;
            float3 ViewSpaceLightPosition = ViewSpacePositionAndRadiusBuffer.PackedData[LocalLightIndex].xyz;
            float LightRadius = ViewSpacePositionAndRadiusBuffer.PackedData[LocalLightIndex].w;

            //float3 ViewSpaceLightPosition = mul(View.WorldToView, float4(LocalLightData.LightPositionAndInvRadius.xyz, 1)).xyz;
            
            float BoxDistanceSq = ComputeSquaredDistanceFromBoxToPoint(ViewTileCenter, ViewTileExtent, ViewSpaceLightPosition);
#if DISABLE_CULLS
            BoxDistanceSq = 0;
#endif
            if (BoxDistanceSq < LightRadius * LightRadius)
            {
				bool bPassSpotlightTest = true;
				{
                    float4 ViewSpaceDirAndPreprocAngle = ViewSpaceDirAndPreprocAngleBuffer.PackedData[LocalLightIndex];
					float TanConeAngle = ViewSpaceDirAndPreprocAngle.w;
                
					if (TanConeAngle > 0.0f)
					{
						//float3 ViewSpaceLightDirection = -ViewSpaceDirAndPreprocAngle.xyz;
						float3 ViewSpaceLightDirection = ViewSpaceDirAndPreprocAngle.xyz;
						bPassSpotlightTest = !IsAabbOutsideInfiniteAcuteConeApprox(ViewSpaceLightPosition, ViewSpaceLightDirection, TanConeAngle, ViewTileCenter, ViewTileExtent);
					}
				}
#if DISABLE_CULLS
                bPassSpotlightTest = true;
#endif
                if (bPassSpotlightTest)
				{
					uint NextLink;
					InterlockedAdd(RWNextCulledLightLink[0], 1U, NextLink);

					if (NextLink < NumAvailableLinks)
					{
						uint PreviousLink;
                        InterlockedExchange(RWStartGridOffset[GridIndex], NextLink, PreviousLink);
                        RWCulledLightLink[NextLink * LIGHT_GRID_LINK_STRIDE + 0] = LocalLightIndex;
                        RWCulledLightLink[NextLink * LIGHT_GRID_LINK_STRIDE + 1] = PreviousLink;
                    }

					uint CulledLightIndex;
					InterlockedAdd(RWNumCulledLightsGrid[GridIndex], 1U, CulledLightIndex);
                    
					if (CulledLightIndex < LightGrid.MaxCulledLightsPerCell)
					{
                        RWCulledLightsGrid[GridIndex * LightGrid.MaxCulledLightsPerCell + CulledLightIndex] = LocalLightIndex;
                    }
				}
//              if (bPassSpotlightTest)
//				{
//
//#if USE_LINKED_CULL_LIST
//					uint NextLink;
//					InterlockedAdd(RWNextCulledLightLink[0], 1U, NextLink);
//
//					if (NextLink < NumAvailableLinks)
//					{
//						uint PreviousLink;
//						InterlockedExchange(RWStartOffsetGrid[GridIndex], NextLink, PreviousLink);
//						RWCulledLightLinks[NextLink * LIGHT_LINK_STRIDE + 0] = LocalLightIndex;
//						RWCulledLightLinks[NextLink * LIGHT_LINK_STRIDE + 1] = PreviousLink;
//					}
//
//#else
//                    InterlockedExchange()
//                    
//					uint CulledLightIndex;
//					InterlockedAdd(RWNumCulledLightsGrid[GridIndex * NUM_CULLED_LIGHTS_GRID_STRIDE + 0], 1U, CulledLightIndex);
//					RWNumCulledLightsGrid[GridIndex * NUM_CULLED_LIGHTS_GRID_STRIDE + 1] = GridIndex * ForwardLightData.MaxCulledLightsPerCell;
//
//					if (CulledLightIndex < ForwardLightData.MaxCulledLightsPerCell)
//					{
//						RWCulledLightDataGrid[GridIndex * ForwardLightData.MaxCulledLightsPerCell + CulledLightIndex] = LocalLightIndex;
//					}
//#endif
//				}
            }
        }
    }
}