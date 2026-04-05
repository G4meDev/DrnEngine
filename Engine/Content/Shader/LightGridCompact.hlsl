
#include "Common.hlsl"

#define THREADGROUP_SIZE 4

struct Resources
{
    uint ViewIndex;
    uint LightGridIndex;
};
ConstantBuffer<Resources> BindlessResources : register(b0);

void CompactReverseLinkedList(uint GridIndex, bool bThreadValid, uint SceneMax, Buffer<uint> CulledLightLink, Buffer<uint> StartGridOffset, RWBuffer<uint> RWNextCulledLightData, RWBuffer<uint> RWNumCulledLightsGrid, RWBuffer<uint> RWCulledLightDataGrid)
{
    uint NumCulledLights = 0;
    uint StartLinkOffset = 0;
    uint LinkOffset = 0;
    uint CulledLightDataStart = 0;

    if (bThreadValid)
    {
        StartLinkOffset = StartGridOffset[GridIndex];
        LinkOffset = StartLinkOffset;

        while (LinkOffset != 0xFFFFFFFF && NumCulledLights < SceneMax)
        {
            NumCulledLights++;
            LinkOffset = CulledLightLink[LinkOffset * LIGHT_GRID_LINK_STRIDE + 1];
        }
    }
    
    InterlockedAdd(RWNextCulledLightData[0], NumCulledLights, CulledLightDataStart);
    
    if (bThreadValid)
    {
        RWNumCulledLightsGrid[GridIndex * LIGHT_GRID_LINK_STRIDE + 0] = NumCulledLights;
        RWNumCulledLightsGrid[GridIndex * LIGHT_GRID_LINK_STRIDE + 1] = CulledLightDataStart;
	
        LinkOffset = StartLinkOffset;
        uint CulledLightIndex = 0;

        while (LinkOffset != 0xFFFFFFFF && CulledLightIndex < NumCulledLights)
        {
            RWCulledLightDataGrid[CulledLightDataStart + NumCulledLights - CulledLightIndex - 1] = CulledLightLink[LinkOffset * LIGHT_GRID_LINK_STRIDE + 0];
            CulledLightIndex++;
            LinkOffset = CulledLightLink[LinkOffset * LIGHT_GRID_LINK_STRIDE + 1];
        }
    }
}

[numthreads(THREADGROUP_SIZE, THREADGROUP_SIZE, THREADGROUP_SIZE)]
void Main_CS(uint3 GroupId : SV_GroupID, uint3 DispatchThreadId : SV_DispatchThreadID, uint3 GroupThreadId : SV_GroupThreadID, uint GroupIndex : SV_GroupIndex)
{
    ConstantBuffer<ViewBuffer> View = ResourceDescriptorHeap[BindlessResources.ViewIndex];
    ConstantBuffer<LightGridData> LightGrid = ResourceDescriptorHeap[BindlessResources.LightGridIndex];
    
    Buffer<uint> CulledLightLink = ResourceDescriptorHeap[LightGrid.CulledLightLinkIndex];
    Buffer<uint> StartGridOffset = ResourceDescriptorHeap[LightGrid.StartGridOffsetIndex];
    
    RWBuffer<uint> RWNextCulledLightData = ResourceDescriptorHeap[LightGrid.RWNextCulledLightDataIndex];
    RWBuffer<uint> RWNumCulledLightsGrid = ResourceDescriptorHeap[LightGrid.RWLightGridNumOffsetIndex];
    RWBuffer<uint> RWCulledLightDataGrid = ResourceDescriptorHeap[LightGrid.RWLightGridLinkListIndex];
    
    uint3 GridCoordinate = DispatchThreadId;

    bool bThreadValid = all(GridCoordinate < LightGrid.CulledGridSize);
	
    uint GridIndex = (GridCoordinate.z * LightGrid.CulledGridSize.y + GridCoordinate.y) * LightGrid.CulledGridSize.x + GridCoordinate.x;

	// Compact lights
    CompactReverseLinkedList(GridIndex, bThreadValid, LightGrid.NumCulledLights, CulledLightLink, StartGridOffset, RWNextCulledLightData, RWNumCulledLightsGrid, RWCulledLightDataGrid);

	// Compact reflection captures
    //CompactReverseLinkedList(ForwardLightData.NumGridCells + GridIndex, ForwardLightData.NumReflectionCaptures, bThreadValid);
}