
#include "Common.hlsl"

struct Resources
{
    uint ViewBufferIndex;
    uint DownSampleBufferIndex;
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

struct DownSampleData
{
    float4 ParentSizeAndInvSize;

    uint ParentTexture;
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

#define DOWNSAMPLE_QUALITY 1

float3 Main_PS(PixelShaderInput IN) : SV_Target
{
    ConstantBuffer<StaticSamplers> StaticSamplers = ResourceDescriptorHeap[BindlessResources.StaticSamplerBufferIndex];
    ConstantBuffer<DownSampleData> DownSampleBuffer = ResourceDescriptorHeap[BindlessResources.DownSampleBufferIndex];
    
    Texture2D ParentTexture = ResourceDescriptorHeap[DownSampleBuffer.ParentTexture];
    SamplerState LinearSampler = ResourceDescriptorHeap[StaticSamplers.LinearClampIndex];
    
    float3 Result = 0;
    
#if DOWNSAMPLE_QUALITY == 0
    float3 Result = ParentTexture.Sample(LinearSampler, IN.UV).xyz;
#else
    float2 UVs[4];

    UVs[0] = IN.UV + DownSampleBuffer.ParentSizeAndInvSize.zw * float2(-1, -1);
    UVs[1] = IN.UV + DownSampleBuffer.ParentSizeAndInvSize.zw * float2(1, -1);
    UVs[2] = IN.UV + DownSampleBuffer.ParentSizeAndInvSize.zw * float2(-1, 1);
    UVs[3] = IN.UV + DownSampleBuffer.ParentSizeAndInvSize.zw * float2(1, 1);

    float3 Sample[4];

    [unroll]
    for (uint i = 0; i < 4; ++i)
    {
        //float2 UV = clamp(UV, Input_UVViewportBilinearMin, Input_UVViewportBilinearMax);
        Sample[i] = ParentTexture.Sample(LinearSampler, UVs[i]).xyz;
    }

    Result = (Sample[0] + Sample[1] + Sample[2] + Sample[3]) * 0.25f;

    Result.rgb = max(0, Result);
#endif

    return Result;
}