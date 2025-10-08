
#include "Common.hlsl"

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
    float4 SizeAndInvSize;

    uint SampleTexture;
    uint AddtiveTexture;
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
    for (int i = -8; i < 8; i++)
    {
        float2 UV = IN.UV;
        
#if BLOOM_Y
        UV += i * float2(0, BloomBuffer.SizeAndInvSize.w);
#else
        UV += i * float2(BloomBuffer.SizeAndInvSize.z, 0);
#endif
        
        Result += SampleTexture.Sample(LinearSampler, UV).xyz;
    }
    
    Result /= 17.0f;
    
#ifndef BLOOM_Y
    Result /= 20;
#endif
    
#if BLOOM_ADDTIVE
    Texture2D AddtiveTexture = ResourceDescriptorHeap[BloomBuffer.AddtiveTexture];
    Result += AddtiveTexture.Sample(LinearSampler, IN.UV).xyz;
#endif

    return Result;
}