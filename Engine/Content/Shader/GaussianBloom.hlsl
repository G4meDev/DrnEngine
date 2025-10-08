
#include "Common.hlsl"

#ifndef STATIC_SAMPLE_COUNT
	#error STATIC_SAMPLE_COUNT is undefined
#endif

#define PACKED_STATIC_SAMPLE_COUNT ((STATIC_SAMPLE_COUNT + 1) / 2)

struct Resources
{
    uint ViewBufferIndex;
    uint BloomBufferIndex;
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

struct BloomData
{
    uint SampleTexture;
    uint AddtiveTexture;
    
    float2 Pad;
    
    float4 SampleOffsetWeights[PACKED_STATIC_SAMPLE_COUNT];
};

struct StaticSamplers
{
    uint LinearSamplerIndex;
    uint PointSamplerIndex;
    uint LinearCmpSamplerIndex;
    uint LinearClampIndex;
    uint PointClampIndex;
};

struct VertexInputPosUV
{
    float3 Position : POSITION;
    float2 UV : TEXCOORD;
};

struct VertexShaderOutput
{
    float2 UV : TEXCOORD0;
    float4 Position : SV_Position;
};

VertexShaderOutput Main_VS(VertexInputPosUV IN)
{
    VertexShaderOutput OUT;

    ConstantBuffer<ViewBuffer> View = ResourceDescriptorHeap[BindlessResources.ViewBufferIndex];
    
    OUT.Position = mul(View.LocalToCameraView, float4(IN.Position, 1.0f));
    OUT.Position.z = 0;
    OUT.UV = IN.UV;

    return OUT;
}

struct PixelShaderInput
{
    float2 UV : TEXCOORD0;
};

//#define BLOOM_Y 1

float3 Main_PS(PixelShaderInput IN) : SV_Target
{
    ConstantBuffer<StaticSamplers> StaticSamplers = ResourceDescriptorHeap[BindlessResources.StaticSamplerBufferIndex];
    ConstantBuffer<BloomData> BloomBuffer = ResourceDescriptorHeap[BindlessResources.BloomBufferIndex];
    
    Texture2D SampleTexture = ResourceDescriptorHeap[BloomBuffer.SampleTexture];
    SamplerState LinearSampler = ResourceDescriptorHeap[StaticSamplers.LinearClampIndex];
    
    float3 Result = 0;
    
    [unroll]
    for (int SampleIndex = 0; SampleIndex < PACKED_STATIC_SAMPLE_COUNT - 1; SampleIndex++)
    {
        float4 SampleOffsetWeights = BloomBuffer.SampleOffsetWeights[SampleIndex];
        
        float SampleWeight_1 = SampleOffsetWeights.y;
        float SampleWeight_2 = SampleOffsetWeights.w;

        float2 SampleOffset_1;
        float2 SampleOffset_2;
        
#if BLOOM_Y 
        SampleOffset_1 = float2(0, SampleOffsetWeights.x);
        SampleOffset_2 = float2(0, SampleOffsetWeights.z);
#else
        SampleOffset_1 = float2(SampleOffsetWeights.x, 0);
        SampleOffset_2 = float2(SampleOffsetWeights.z, 0);
#endif
        
        Result += SampleTexture.Sample(LinearSampler, IN.UV + SampleOffset_1).xyz * SampleWeight_1;
        Result += SampleTexture.Sample(LinearSampler, IN.UV + SampleOffset_2).xyz * SampleWeight_2;
    }
    
    float4 SampleOffsetWeights = BloomBuffer.SampleOffsetWeights[PACKED_STATIC_SAMPLE_COUNT - 1];
    float SampleWeight = SampleOffsetWeights.y;
    float2 SampleOffset;
    
#if BLOOM_Y 
    SampleOffset = float2(0, SampleOffsetWeights.x);
#else
    SampleOffset = float2(SampleOffsetWeights.x, 0);
#endif
    
    Result += SampleTexture.Sample(LinearSampler, IN.UV + SampleOffset).xyz * SampleWeight;
    
#if BLOOM_ADDTIVE
    Texture2D AddtiveTexture = ResourceDescriptorHeap[BloomBuffer.AddtiveTexture];
    Result += AddtiveTexture.Sample(LinearSampler, IN.UV).xyz;
#endif

    return Result;
}