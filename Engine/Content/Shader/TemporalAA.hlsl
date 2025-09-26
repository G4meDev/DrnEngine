
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

[numthreads(8, 8, 1)]
void Main_CS(uint2 GroupId : SV_GroupID, uint GroupThreadIndex : SV_GroupIndex)
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
    
    uint2 GroupThreadId = uint2(GroupThreadIndex % 8, GroupThreadIndex / 8);
    uint2 DispatchThreadId = 8 * GroupId + GroupThreadId;
    uint2 OutputPixelPos = DispatchThreadId;
    
    float2 BufferUV = (DispatchThreadId + 0.5f) * View.InvSize;
    
    float2 Velocity = VelocityTexture.Sample(PointSampler, BufferUV).xy;
    Velocity = (Velocity - 0.5f) * 4;
    
    float2 PrevUV = BufferUV - (Velocity * View.InvSize * 0.25);
    bool bValid = PrevUV.x >= 0 && PrevUV.x <= 1 && PrevUV.y >= 0 && PrevUV.y <= 1;
    
    float3 DeferredColor = DeferredTexture.Sample(PointSampler, BufferUV).xyz;
    float3 Result;

    if(bValid)
    {
        //float3 HistoryColor = HistoryTexture[OutputPixelPos].xyz;
        
        //uint2 HistoryPos = PrevUV * View.RenderSize;
        //float3 HistoryColor = HistoryTexture[HistoryPos].xyz;
        
        float3 HistoryColor = HistoryTexture.Sample(PointSampler, PrevUV).xyz;
       
        Result = lerp(DeferredColor, HistoryColor, 0.9f);
        //Result = 0;
    }
    
    else
    {
        Result = DeferredColor;
    }
    
    //Result = float3(PrevUV.xy, 0);
    
    TargetTexture[OutputPixelPos] = float4(Result, 1);
}