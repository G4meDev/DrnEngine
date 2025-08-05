#define GROUP_TILE_SIZE 8

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

struct StaticSamplers
{
    uint LinearSamplerIndex;
    uint PointSamplerIndex;
};

[numthreads(GROUP_TILE_SIZE, GROUP_TILE_SIZE, 1)]
void Main_CS(uint2 GroupId : SV_GroupID, uint GroupThreadIndex : SV_GroupIndex)
{
    ConstantBuffer<StaticSamplers> StaticSamplers = ResourceDescriptorHeap[BindlessResources.StaticSamplerBufferIndex];
    SamplerState PointSampler = ResourceDescriptorHeap[StaticSamplers.PointSamplerIndex];
    
    Texture2D<float> ParentTexture = ResourceDescriptorHeap[BindlessResources.ParentTextureIndex];
    RWTexture2D<float> Output_0 = ResourceDescriptorHeap[BindlessResources.WriteTextureIndex_1];
    
    uint2 GroupThreadId = uint2(GroupThreadIndex % GROUP_TILE_SIZE, GroupThreadIndex / GROUP_TILE_SIZE);
    uint2 DispatchThreadId = GROUP_TILE_SIZE * GroupId + GroupThreadId;
    
    uint2 OutputPixelPos = DispatchThreadId;
    
    float2 UV = (float2)OutputPixelPos * BindlessResources.InvSize;
    float ParentSample = ParentTexture.Sample(PointSampler, UV);
    
    Output_0[OutputPixelPos] = ParentSample;
    //Output_0[OutputPixelPos] = UV.x;
}