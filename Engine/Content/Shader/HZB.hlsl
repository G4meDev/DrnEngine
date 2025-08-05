#define GROUP_TILE_SIZE 8

struct Resources
{
    uint ViewBufferIndex;
    //uint DispatchDataIndex;
    uint WriteTextureIndex;
    uint StaticSamplerBufferIndex;
};

ConstantBuffer<Resources> BindlessResources : register(b0);

struct DispatchData
{
    float2 DispatchIdToUV;
    float2 InvSize;
};

struct StaticSamplers
{
    uint LinearSamplerIndex;
    uint PointSamplerIndex;
};

[numthreads(GROUP_TILE_SIZE, GROUP_TILE_SIZE, 1)]
void Main_CS(uint2 GroupId : SV_GroupID, uint GroupThreadIndex : SV_GroupIndex)
{
    RWTexture2D<float> Output_0 = ResourceDescriptorHeap[BindlessResources.WriteTextureIndex];
    
    uint2 GroupThreadId = uint2(GroupThreadIndex % GROUP_TILE_SIZE, GroupThreadIndex / GROUP_TILE_SIZE);
    uint2 DispatchThreadId = GROUP_TILE_SIZE * GroupId + GroupThreadId;
    
    uint2 OutputPixelPos = DispatchThreadId;
    
    Output_0[OutputPixelPos] = 0.23f;
}

//float4 Main_PS(PixelShaderInput IN) : SV_Target
//{
//    ConstantBuffer<StaticSamplers> StaticSamplers = ResourceDescriptorHeap[BindlessResources.StaticSamplerBufferIndex];
//    
//    Texture2D HdrImage = ResourceDescriptorHeap[BindlessResources.DeferredColorIndex];
//    SamplerState LinearSampler = ResourceDescriptorHeap[StaticSamplers.LinearSamplerIndex];
//    
//    float Exposure = 0.2f;
//    float Gamma = 2.2f;
//    
//    float3 HdrColor = HdrImage.Sample(LinearSampler, IN.UV).xyz;
//    float3 Mapped = float3(1.0f, 1.0f, 1.0f) - exp(-HdrColor * Exposure.xxx);
//    
//    float a = 1.0f / Gamma;
//    Mapped = pow(Mapped, float3(a.xxx));
//    
//    return float4(Mapped, 1);
//}