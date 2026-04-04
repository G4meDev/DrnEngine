
#include "Common.hlsl"

struct Resources
{
    uint NumEntries;
    uint ClearValue;
    uint UAVIndex;
};

ConstantBuffer<Resources> BindlessResources : register(b0);

[numthreads(64, 1, 1)]
void Main_CS(uint3 DispatchThreadId : SV_DispatchThreadID)
{
    RWBuffer<uint> UAV = ResourceDescriptorHeap[BindlessResources.UAVIndex];
    
    if (DispatchThreadId.x < BindlessResources.NumEntries)
    {
        UAV[DispatchThreadId.x] = BindlessResources.ClearValue;
    }
}