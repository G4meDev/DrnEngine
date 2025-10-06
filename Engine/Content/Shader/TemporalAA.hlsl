
#include "Common.hlsl"

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
    float2 JitterOffset;
    
    float2 PrevJitterOffset;
    float2 Pad_1;
    
    matrix ClipToPreviousClip;
};

struct TAAData
{
    uint DeferredColorTexture;
    uint VelocityTexture;
    uint HistoryTexture;
    uint TargetTexture;
    
    uint DepthTexture;
    float CcurrentFrameWeight;
    float CcurrentFrameVelocityWeight;
    float CcurrentFrameVelocityMultiplier;
};

struct StaticSamplers
{
    uint LinearSamplerIndex;
    uint PointSamplerIndex;
    uint LinearCmpSamplerIndex;
    uint LinearClampIndex;
    uint PointClampIndex;
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

float SampleDepthTexture(Texture2D DepthTexture, SamplerState Sampler, float2 UV, int2 PixelOffset)
{
    return DepthTexture.SampleLevel(Sampler, UV, 0, PixelOffset).r;
}

#define CROSS_DIST 1

#if CROSS_DIST
static const int2 CrossOffset[4] =
{
    int2(-CROSS_DIST, -CROSS_DIST),
    int2(CROSS_DIST, -CROSS_DIST),
    int2(-CROSS_DIST, CROSS_DIST),
    int2(CROSS_DIST, CROSS_DIST),
};
#endif

#define CLAMP_COUNT 4
#define Clamp_DIST 1

#if CLAMP_COUNT
static const int2 ClampOffset[4] =
{
    int2(0, -Clamp_DIST),
    int2(Clamp_DIST, 0),
    int2(0, Clamp_DIST),
    int2(-Clamp_DIST, 0),
};
#endif

[numthreads(8, 8, 1)]
void Main_CS(uint2 DispatchThreadId : SV_DispatchThreadID, uint2 GroupId : SV_GroupID, uint GroupThreadIndex : SV_GroupIndex)
{
    ConstantBuffer<ViewBuffer> View = ResourceDescriptorHeap[BindlessResources.ViewBufferIndex];
    ConstantBuffer<TAAData> TAABuffer = ResourceDescriptorHeap[BindlessResources.TAABufferIndex];
    ConstantBuffer<StaticSamplers> StaticSamplers = ResourceDescriptorHeap[BindlessResources.StaticSamplerBufferIndex];
    
    Texture2D DeferredTexture = ResourceDescriptorHeap[TAABuffer.DeferredColorTexture];
    Texture2D DepthTexture = ResourceDescriptorHeap[TAABuffer.DepthTexture];
    Texture2D VelocityTexture = ResourceDescriptorHeap[TAABuffer.VelocityTexture];
    Texture2D HistoryTexture = ResourceDescriptorHeap[TAABuffer.HistoryTexture];
    RWTexture2D<float4> TargetTexture = ResourceDescriptorHeap[TAABuffer.TargetTexture];

    SamplerState LinearSampler = ResourceDescriptorHeap[StaticSamplers.LinearClampIndex];
    SamplerState PointSampler = ResourceDescriptorHeap[StaticSamplers.PointClampIndex];
    
    float2 BufferUV = (DispatchThreadId + 0.5f) * View.InvSize;
    uint2 OutputPixelPos = DispatchThreadId;
    
    float PixelDepth = SampleDepthTexture(DepthTexture, PointSampler, BufferUV, int2(0, 0));
    float2 VelocityOffset = float2(0.0, 0.0);
    
#if CROSS_DIST
    float4 Depths;
    Depths.x = SampleDepthTexture(DepthTexture, PointSampler, BufferUV, CrossOffset[0]);
    Depths.y = SampleDepthTexture(DepthTexture, PointSampler, BufferUV, CrossOffset[1]);
    Depths.z = SampleDepthTexture(DepthTexture, PointSampler, BufferUV, CrossOffset[2]);
    Depths.w = SampleDepthTexture(DepthTexture, PointSampler, BufferUV, CrossOffset[3]);
    
    int index = -1;
    float MaxDepth = 0; // nearest
    [unroll]
    for (int i = 0; i < 4; i++)
    {
        if(Depths[i] > MaxDepth)
        {
            MaxDepth = Depths[i];
            index = i;
        }
    }
    
    float2 DepthOffset = int2(CrossOffset[index]);
    //DepthOffset = PixelDepth > MaxDepth ? float2(0, 0) : DepthOffset;
    
    VelocityOffset = DepthOffset * View.InvSize;
#endif
    
    //float2 Velocity = VelocityTexture.Sample(PointSampler, BufferUV + VelocityOffset).xy;
    //Velocity = (Velocity - 0.5f) * 4;
    //
    //float2 VeloTemp = Velocity * View.RenderSize;
    //float VeloLen = sqrt(dot(VeloTemp, VeloTemp));
    
    float2 ScreenPos = ViewportUVToScreenPos(BufferUV);
    //float2 PrevScreenPos = ScreenPos - Velocity;
    //float2 PrevUV = ScreenPosToViewportUV(PrevScreenPos);
    
    float4 ThisClip = float4(ScreenPos, PixelDepth, 1);
    float4 PrevClip = mul(View.ClipToPreviousClip, ThisClip);
    float2 PrevScreenPos = PrevClip.xy / PrevClip.w;
    float2 PrevUV = ScreenPosToViewportUV(PrevScreenPos);
    
    float2 Velo = ScreenPos - PrevScreenPos;
    float2 VeloTemp = Velo * View.RenderSize;
    float VeloLen = sqrt(dot(VeloTemp, VeloTemp));
    
    float3 DeferredColor = DeferredTexture.Sample(PointSampler, BufferUV).xyz;
    float3 Result;

    bool OffScreen = max(abs(PrevScreenPos.x), abs(PrevScreenPos.y)) >= 1.0;
    
    float3 HistoryColor = HistoryTexture.Sample(LinearSampler, PrevUV).xyz;
    
#if CLAMP_COUNT
    float3 BoundsMin = 100000;
    float3 BoundsMax = 0;
        
    [unroll]
    for (int i = 0; i < CLAMP_COUNT; i++)
    {
        float3 Sample = DeferredTexture.Sample(PointSampler, BufferUV, ClampOffset[i]).rgb;
            
        BoundsMin = min(BoundsMin, Sample);
        BoundsMax = max(BoundsMax, Sample);
    }

    HistoryColor = clamp(HistoryColor, BoundsMin, BoundsMax);
    
#endif
    
    //BlendFactor = lerp(0.04, 0.2f, saturate(VeloLen * 0.025f));
    float BlendFactor = lerp(TAABuffer.CcurrentFrameWeight, TAABuffer.CcurrentFrameVelocityWeight, saturate(VeloLen * TAABuffer.CcurrentFrameVelocityMultiplier));
    
    if(OffScreen)
    {
        BlendFactor = 1.0f;
    }
    
    Result = lerp(HistoryColor, DeferredColor, BlendFactor);
    
    TargetTexture[OutputPixelPos] = float4(Result, 1);
}