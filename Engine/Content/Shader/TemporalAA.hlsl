
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
    uint VelocityTexture;
    uint HistoryTexture;
    uint TargetTexture;
};

struct StaticSamplers
{
    uint LinearSamplerIndex;
    uint PointSamplerIndex;
};

float3 Luminance(float3 LinearColor)
{
    return dot(LinearColor, float3(0.3, 0.59, 0.11));
}

float2 ViewportUVToScreenPos(float2 ViewportUV)
{
    return float2(2 * ViewportUV.x - 1, 1 - 2 * ViewportUV.y);
}

float2 ScreenPosToViewportUV(float2 ScreenPos)
{
    return float2(0.5 + 0.5 * ScreenPos.x, 0.5 - 0.5 * ScreenPos.y);
}

[numthreads(8, 8, 1)]
void Main_CS(uint2 DispatchThreadId : SV_DispatchThreadID, uint2 GroupId : SV_GroupID, uint GroupThreadIndex : SV_GroupIndex)
{
    ConstantBuffer<ViewBuffer> View = ResourceDescriptorHeap[BindlessResources.ViewBufferIndex];
    ConstantBuffer<TAAData> TAABuffer = ResourceDescriptorHeap[BindlessResources.TAABufferIndex];
    ConstantBuffer<StaticSamplers> StaticSamplers = ResourceDescriptorHeap[BindlessResources.StaticSamplerBufferIndex];
    
    Texture2D DeferredTexture = ResourceDescriptorHeap[TAABuffer.DeferredColorTexture];
    Texture2D VelocityTexture = ResourceDescriptorHeap[TAABuffer.VelocityTexture];
    Texture2D HistoryTexture = ResourceDescriptorHeap[TAABuffer.HistoryTexture];
    RWTexture2D<float4> TargetTexture = ResourceDescriptorHeap[TAABuffer.TargetTexture];

    SamplerState LinearSampler = ResourceDescriptorHeap[StaticSamplers.LinearSamplerIndex];
    SamplerState PointSampler = ResourceDescriptorHeap[StaticSamplers.PointSamplerIndex];
    
    float2 BufferUV = (DispatchThreadId + 0.5f) * View.InvSize;
    uint2 OutputPixelPos = DispatchThreadId;
    
    float2 Velocity = VelocityTexture.Sample(PointSampler, BufferUV).xy;
    Velocity = (Velocity - 0.5f) * 4;
    
    float2 ScreenPos = ViewportUVToScreenPos(BufferUV);
    float2 PrevScreenPos = ScreenPos - Velocity;
    float2 PrevUV = ScreenPosToViewportUV(PrevScreenPos);
    
    float3 DeferredColor = DeferredTexture.Sample(PointSampler, BufferUV).xyz;
    float3 Result;

    bool OffScreen = max(abs(PrevScreenPos.x), abs(PrevScreenPos.y)) >= 1.0;
    if (!OffScreen)
    {
        float3 HistoryColor = HistoryTexture.Sample(PointSampler, PrevUV).xyz;
        Result = lerp(DeferredColor, HistoryColor, 0.5f);
    }
    
    else
    {
        Result = DeferredColor;
    }
    
    TargetTexture[OutputPixelPos] = float4(Result, 1);
}