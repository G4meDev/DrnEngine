
#include "Common.hlsl"

#define THREADGROUP_SIZE 4

struct Resources
{
    uint ViewBufferIndex;
    uint ParentTextureIndex;
    uint StaticSamplerBufferIndex;
    uint WriteTextureIndex_1;
    
    uint WriteTextureIndex_2;
    uint WriteTextureIndex_3;
    uint WriteTextureIndex_4;
    uint Pad_1;
    
    float4 DispatchIdToUV;
    float2 InvSize;
};

ConstantBuffer<Resources> BindlessResources : register(b0);

[numthreads(THREADGROUP_SIZE, THREADGROUP_SIZE, THREADGROUP_SIZE)]
void Main_CS(uint2 GroupId : SV_GroupID, uint GroupThreadIndex : SV_GroupIndex)
{
    ConstantBuffer<StaticSamplers> StaticSamplers = ResourceDescriptorHeap[BindlessResources.StaticSamplerBufferIndex];
    SamplerState PointSampler = ResourceDescriptorHeap[StaticSamplers.PointClampIndex];
    
    
}