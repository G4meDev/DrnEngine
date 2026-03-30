
#include "Common.hlsl"

struct Resources
{
    uint ViewBufferIndex;
    uint TAABufferIndex;
    uint StaticSamplerBufferIndex;
};

ConstantBuffer<Resources> BindlessResources : register(b0);

struct TAAData
{
    float4 SampleWeights[3];
    float4 PlusWeights[2];
    
    //float SampleWeights[12];
    //float PlusWeights[8];

    uint DeferredColorTexture;
    uint VelocityTexture;
    uint HistoryTexture;
    uint TargetTexture;
    
    uint DepthTexture;
    float CcurrentFrameWeight;
    float CcurrentFrameVelocityWeight;
    float CcurrentFrameVelocityMultiplier;
    
};

float3 Luminance(float3 LinearColor)
{
    return dot(LinearColor, float3(0.3, 0.59, 0.11));
}

float SampleDepthTexture(Texture2D DepthTexture, SamplerState Sampler, float2 UV, int2 PixelOffset)
{
    return DepthTexture.SampleLevel(Sampler, UV, 0, PixelOffset).r;
}

struct FTAAIntermediaryResult
{
    float4 Filtered;
};

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

static const int2 kOffsets3x3[9] =
{
    int2(-1, -1),
	int2(0, -1),
	int2(1, -1),
	int2(-1, 0),
	int2(0, 0), // K
	int2(1, 0),
	int2(-1, 1),
	int2(0, 1),
	int2(1, 1),
};
	
static const uint kSquareIndexes3x3[9] = { 0, 1, 2, 3, 4, 5, 6, 7, 8 };
static const uint kPlusIndexes3x3[5] = { 1, 3, 4, 5, 7 };

float HistoryClip(float3 History, float3 Filtered, float3 NeighborMin, float3 NeighborMax)
{
    float3 BoxMin = NeighborMin;
    float3 BoxMax = NeighborMax;

    float3 RayOrigin = History;
    float3 RayDir = Filtered - History;
    RayDir = select(abs(RayDir) < (1.0 / 65536.0), (1.0 / 65536.0), RayDir);
    float3 InvRayDir = rcp(RayDir);

    float3 MinIntersect = (BoxMin - RayOrigin) * InvRayDir;
    float3 MaxIntersect = (BoxMax - RayOrigin) * InvRayDir;
    float3 EnterIntersect = min(MinIntersect, MaxIntersect);
    return max3(EnterIntersect.x, EnterIntersect.y, EnterIntersect.z);
}

float4 SampleCachedSceneColorTexture(Texture2D SceneColorTexture, int2 ScreenPixel, int2 PixelOffset)
{
    int2 Coord = ScreenPixel + PixelOffset;
    return SceneColorTexture[Coord];
}

void FilterCurrentFrameInputSamples(in TAAData Data, Texture2D SceneColorTexture, int2 ScreenPixel, inout FTAAIntermediaryResult Result)
{
#define AA_SAMPLES 9
    
    Result.Filtered = 0;
    
    [unroll]
    for (uint i = 0; i < AA_SAMPLES; i++)
    {
        #if AA_SAMPLES == 9
            const uint SampleIndexes[9] = kSquareIndexes3x3;
        #elif AA_SAMPLES == 5
		    const uint SampleIndexes[5] = kPlusIndexes3x3;
        #endif
        
		const uint SampleIndex = SampleIndexes[i];
		int2 SampleOffset = kOffsets3x3[SampleIndex];
		//float2 fSampleOffset = float2(SampleOffset);
        
        const int j = i / 4;
        const int k = i % 4;
        
        #if AA_SAMPLES == 9
            //float SampleSpatialWeight = Data.SampleWeights[i];
            float SampleSpatialWeight = Data.SampleWeights[j][k];

		#elif AA_SAMPLES == 5
            //float SampleSpatialWeight = Data.PlusWeights[i];
            float SampleSpatialWeight = Data.PlusWeights[j][k];

        #else
            #error unsupported
        #endif

        float4 Sample = SampleCachedSceneColorTexture(SceneColorTexture, ScreenPixel, SampleOffset);
        Result.Filtered += Sample * SampleSpatialWeight;
    }
}

void ComputeNeighborhoodBoundingbox(FTAAIntermediaryResult IntermediaryResult, Texture2D SceneColorTexture, int2 ScreenPixel, out float3 BoundsMin, out float3 BoundsMax)
{
    const uint kNeighborsCount = 9;
    float3 Neighbors[kNeighborsCount];
    [unroll]
    for (uint i = 0; i < kNeighborsCount; i++)
    {
        Neighbors[i] = SampleCachedSceneColorTexture(SceneColorTexture, ScreenPixel, kOffsets3x3[i]).rgb;
    }
    
    BoundsMin = 100000;
    BoundsMax = 0;

#if 0
    [unroll]
    for (int i = 0; i < CLAMP_COUNT; i++)
    {
        float3 Sample = SampleCachedSceneColorTexture(SceneColorTexture, ScreenPixel, ClampOffset[i]).rgb;
        BoundsMin = min(BoundsMin, Sample);
        BoundsMax = max(BoundsMax, Sample);
    }
#elif 1
    BoundsMin = min(min(Neighbors[1], Neighbors[3]), Neighbors[4]);
    BoundsMin = min(min(BoundsMin, Neighbors[5]), Neighbors[7]);

    BoundsMax = max(max(Neighbors[1], Neighbors[3]), Neighbors[4]);
    BoundsMax = max(max(BoundsMax, Neighbors[5]), Neighbors[7]);
		
	#if AA_SAMPLES == 5
	{
		float2 PPCo = InputParams.ViewportUV * InputViewSize.xy + TemporalJitterPixels;
		float2 PPCk = floor(PPCo) + 0.5;
		float2 dKO = PPCo - PPCk;
			
		int2 FifthNeighborOffset = SignFastInt(dKO);

		FTAAHistoryPayload FifthNeighbor;
		FifthNeighbor.Color = SampleCachedSceneColorTexture(InputParams, FifthNeighborOffset).Color;
		FifthNeighbor.CocRadius = SampleCachedSceneColorTexture(InputParams, FifthNeighborOffset).CocRadius;
			
		NeighborMin = MinPayload(NeighborMin, FifthNeighbor);
		NeighborMax = MaxPayload(NeighborMax, FifthNeighbor);
	}
	#elif AA_SAMPLES == 9
	{
        float3 NeighborMinPlus = BoundsMin;
        float3 NeighborMaxPlus = BoundsMax;

        BoundsMin = min(min(BoundsMin, Neighbors[0]), Neighbors[2]);
        BoundsMin = min(min(BoundsMin, Neighbors[6]), Neighbors[8]);

        BoundsMax = max(max(BoundsMax, Neighbors[0]), Neighbors[2]);
        BoundsMax = max(max(BoundsMax, Neighbors[6]), Neighbors[8]);
    #endif

    #if 1
        BoundsMin = BoundsMin * 0.5 + NeighborMinPlus * 0.5;
        BoundsMax = BoundsMax * 0.5 + NeighborMaxPlus * 0.5;
    #endif
    }
#else
    #if AA_SAMPLES == 9
        const uint SampleIndexes[9] = kSquareIndexes3x3;
    #elif AA_SAMPLES == 5
		const uint SampleIndexes[5] = kPlusIndexes3x3;
    #else
		#error Unknown number of samples.
    #endif

    float3 m1 = 0;
    float3 m2 = 0;
    [unroll]
    for (uint i = 0; i < AA_SAMPLES; i++)
    {
        float3 SampleColor = Neighbors[SampleIndexes[i]];

        m1 += SampleColor;
        m2 += SampleColor * SampleColor;
    }

    m1 *= (1.0 / AA_SAMPLES);
    m2 *= (1.0 / AA_SAMPLES);

    float3 StdDev = sqrt(abs(m2 - m1 * m1));
    BoundsMin = m1 - 1.25 * StdDev;
    BoundsMax = m1 + 1.25 * StdDev;

    BoundsMin = min(BoundsMin, IntermediaryResult.Filtered.rgb);
    BoundsMax = max(BoundsMax, IntermediaryResult.Filtered.rgb);
#endif
}

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
    float2 NearestBufferUV = BufferUV;
    uint2 OutputPixelPos = DispatchThreadId;
    float2 ScreenPixel = int2(NearestBufferUV * View.RenderSize);
    
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
    
    if(MaxDepth > PixelDepth)
    {
        PixelDepth = MaxDepth;
    }
    
    //float2 DepthOffset = int2(CrossOffset[index]);
    //DepthOffset = PixelDepth > MaxDepth ? float2(0, 0) : DepthOffset;
    //VelocityOffset = DepthOffset * View.InvSize;
#endif
    
    float2 ScreenPos = ViewportUVToScreenPos(BufferUV);
    
    float4 ThisClip = float4(ScreenPos, PixelDepth, 1);
    float4 PrevClip = mul(View.ClipToPreviousClip, ThisClip);
    float2 PrevScreenPos = PrevClip.xy / PrevClip.w;
    float2 PrevUV = ScreenPosToViewportUV(PrevScreenPos);
    
    float2 Velo = ScreenPos - PrevScreenPos;
    float2 VeloTemp = Velo * View.RenderSize;
    
#if 1
    float2 EncodedVelocity = VelocityTexture.SampleLevel(PointSampler, NearestBufferUV + VelocityOffset, 0).rg;
    bool DynamicN = EncodedVelocity.x > 0.0;
    if (DynamicN)
    {
        Velo = DecodeVelocityFromTexture(EncodedVelocity);
        PrevUV = ScreenPosToViewportUV(ScreenPos - Velo);
        VeloTemp = Velo * View.RenderSize;
    }
#endif
    
    float VeloLen = sqrt(dot(VeloTemp, VeloTemp));
    
    float HistoryBlurAmp = 2.0;
    float HistoryBlur = saturate(abs(VeloTemp.x) * HistoryBlurAmp + abs(VeloTemp.y) * HistoryBlurAmp);
    
    float3 DeferredColor = DeferredTexture.Sample(PointSampler, BufferUV).xyz;
    float3 Result;

    bool OffScreen = max(abs(PrevScreenPos.x), abs(PrevScreenPos.y)) >= 1.0;
    
    FTAAIntermediaryResult IntermediaryResult;
    FilterCurrentFrameInputSamples(TAABuffer, DeferredTexture, ScreenPixel, IntermediaryResult);
    
    float4 History = HistoryTexture.Sample(LinearSampler, PrevUV);
    float3 HistoryColor = History.xyz;
    
#if CLAMP_COUNT
    
    float3 BoundsMin;
    float3 BoundsMax;
    ComputeNeighborhoodBoundingbox(IntermediaryResult, DeferredTexture, ScreenPixel, BoundsMin, BoundsMax);

    bool IgnoreHistory = false;
    bool Dynamic4;
#if 1
	{
        bool Dynamic1 = VelocityTexture.SampleLevel(PointSampler, NearestBufferUV, 0, int2(0, -1)).x > 0;
        bool Dynamic3 = VelocityTexture.SampleLevel(PointSampler, NearestBufferUV, 0, int2(-1, 0)).x > 0;
        Dynamic4 = VelocityTexture.SampleLevel(PointSampler, NearestBufferUV, 0).x > 0;
        bool Dynamic5 = VelocityTexture.SampleLevel(PointSampler, NearestBufferUV, 0, int2(1, 0)).x > 0;
        bool Dynamic7 = VelocityTexture.SampleLevel(PointSampler, NearestBufferUV, 0, int2(0, 1)).x > 0;

        bool Dynamic = Dynamic1 || Dynamic3 || Dynamic4 || Dynamic5 || Dynamic7;
        IgnoreHistory = !Dynamic && History.a > 0;
    }
#endif
    
    float LumaHistory = Luma4(HistoryColor);
    float LumaFiltered = Luma4(IntermediaryResult.Filtered.rgb);
    float LumaMin = Luma4(BoundsMin);
    float LumaMax = Luma4(BoundsMax);
    
#define AA_CLIP 1
//#define AA_CLAMP 1
    
#if AA_CLAMP
    HistoryColor = clamp(HistoryColor, BoundsMin, BoundsMax);
#elif AA_CLIP
    float3 TargetColor = 0.5 * (BoundsMin + BoundsMax);
    float ClipBlend = HistoryClip(HistoryColor, TargetColor, BoundsMin, BoundsMax);
	ClipBlend = saturate( ClipBlend );
	HistoryColor = lerp(HistoryColor, TargetColor, ClipBlend);
#endif
#endif
    
    //{
    //    float AddAliasing = saturate(HistoryBlur) * 0.5;
    //    float LumaContrastFactor = 32.0;
    //    float LumaContrast = LumaMax - LumaMin;
    //    AddAliasing = saturate(AddAliasing + rcp(1.0 + LumaContrast * LumaContrastFactor));
    //    IntermediaryResult.Filtered = lerp(IntermediaryResult.Filtered, SampleCachedSceneColorTexture(DeferredTexture, ScreenPixel, int2(0, 0)), AddAliasing);
    //}
    
    //float BlendFactor = lerp(0.04, 0.2f, saturate(VeloLen * 0.025f));
    float BlendFactor = lerp(TAABuffer.CcurrentFrameWeight, TAABuffer.CcurrentFrameVelocityWeight, saturate(VeloLen * TAABuffer.CcurrentFrameVelocityMultiplier));
    BlendFactor = max(BlendFactor, saturate(0.01 * LumaHistory / abs(LumaFiltered - LumaHistory)));
    
    if(OffScreen || IgnoreHistory)
    {
        BlendFactor = 1.0f;
    }
    
    //Result = lerp(HistoryColor, DeferredColor, BlendFactor);
    Result = lerp(HistoryColor, IntermediaryResult.Filtered.rgb, BlendFactor);
    
    //TargetTexture[OutputPixelPos] = float4(Result, 1);
    TargetTexture[OutputPixelPos] = float4(Result, Dynamic4 ? 1 : 0);
}