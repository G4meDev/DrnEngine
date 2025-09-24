
struct Resources
{
    uint ViewBufferIndex;
    uint TAABufferIndex;
    uint StaticSamplerBufferIndex;
};

ConstantBuffer<Resources> BindlessResources : register(b0);

struct ViewBuffer
{
    matrix WorldToView;
    matrix ViewToProjection;
    matrix WorldToProjection;
    matrix ProjectionToView;
    matrix ProjectionToWorld;
    matrix LocalToCameraView;

    uint2 RenderSize;
    float2 InvSize;

    float3 CameraPos;
    float InvTanHalfFov;
		
    float3 CameraDir;
    float Pad_4;

    float4 InvDeviceZToWorldZTransform;
    matrix ViewToWorld;
    matrix ScreenToTranslatedWorld;
    
    uint FrameIndex;
    uint FrameIndexMod8;
};

struct TAAData
{
    uint DeferredColorTexture;
    uint HistoryTexture;
};

struct StaticSamplers
{
    uint LinearSamplerIndex;
    uint PointSamplerIndex;
};

[numthreads(8, 8, 1)]
void Main_CS(uint2 GroupId : SV_GroupID, uint GroupThreadIndex : SV_GroupIndex)
{
    ConstantBuffer<ViewBuffer> View = ResourceDescriptorHeap[BindlessResources.ViewBufferIndex];
    ConstantBuffer<TAAData> TAABuffer = ResourceDescriptorHeap[BindlessResources.TAABufferIndex];
    ConstantBuffer<StaticSamplers> StaticSamplers = ResourceDescriptorHeap[BindlessResources.StaticSamplerBufferIndex];
    
    Texture2D DeferredTexture = ResourceDescriptorHeap[TAABuffer.DeferredColorTexture];
    RWTexture2D<float4> HistoryTexture = ResourceDescriptorHeap[TAABuffer.HistoryTexture];

    SamplerState LinearSampler = ResourceDescriptorHeap[StaticSamplers.LinearSamplerIndex];
    SamplerState PointSampler = ResourceDescriptorHeap[StaticSamplers.PointSamplerIndex];
    
    uint2 GroupThreadId = uint2(GroupThreadIndex % 8, GroupThreadIndex / 8);
    uint2 DispatchThreadId = 8 * GroupId + GroupThreadId;
    uint2 OutputPixelPos = DispatchThreadId;
    
    float2 BufferUV = (DispatchThreadId + 0.5f) * View.InvSize;
    
    float3 DeferredColor = DeferredTexture.Sample(PointSampler, BufferUV).xyz;
    float3 HistoryColor = HistoryTexture[OutputPixelPos].xyz;

    HistoryTexture[OutputPixelPos] = float4(DeferredColor, 1);
}