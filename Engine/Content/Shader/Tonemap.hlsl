
#include "common.hlsl"

struct Resources
{
    uint ViewBufferIndex;
    uint DeferredColorIndex;
    uint StaticSamplerBufferIndex;
    uint BloomIndex;
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

struct StaticSamplers
{
    uint LinearSamplerIndex;
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

float3 ACESFilmic( float3 x, float A, float B, float C, float D, float E, float F )
{
    return ( ( x * ( A * x + C * B ) + D * E ) / ( x * ( A * x + B ) + D * F ) ) - ( E / F );
}

float4 Main_PS(PixelShaderInput IN) : SV_Target
{
    ConstantBuffer<ViewBuffer> View = ResourceDescriptorHeap[BindlessResources.ViewBufferIndex];
    ConstantBuffer<StaticSamplers> StaticSamplers = ResourceDescriptorHeap[BindlessResources.StaticSamplerBufferIndex];
    
    Texture2D HdrImage = ResourceDescriptorHeap[BindlessResources.DeferredColorIndex];
    Texture2D BloomImage = ResourceDescriptorHeap[BindlessResources.BloomIndex];
    SamplerState LinearSampler = ResourceDescriptorHeap[StaticSamplers.LinearSamplerIndex];
    
    float A = 0.22f;
    float B = 0.30f;
    float C = 0.10f;
    float D = 0.20f;
    float E = 0.01f;
    float F = 0.30f;
    float LinearWhite = 11.02f;
    
    float Exposure = 2.0f;
    float Gamma = 2.2f;
    
    float3 HdrColor = HdrImage.Sample(LinearSampler, IN.UV).xyz;
    float3 Bloom = BloomImage.Sample(LinearSampler, IN.UV).xyz;
    HdrColor += Bloom;
    HdrColor *= exp2(Exposure);
    
    float3 SDR = ACESFilmic(HdrColor, A, B, C, D, E, F) /
              ACESFilmic(LinearWhite, A, B, C, D, E, F);
    
    float3 Result = pow(abs(SDR), 1.0f / Gamma);
    
    float2 DitherUV = IN.UV * View.RenderSize;
    float3 Dither = float3(InterleavedGradientNoise(DitherUV, View.FrameIndex), InterleavedGradientNoise(DitherUV, View.FrameIndex * 117), InterleavedGradientNoise(DitherUV, View.FrameIndex * 53));
    Dither /= 255.0f;
    Result += Dither;
    
    //Result = Dither;
    
    return float4(Result, 1);
}