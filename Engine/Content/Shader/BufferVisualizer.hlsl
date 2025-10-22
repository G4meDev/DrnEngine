#include "Common.hlsl"

struct Resources
{
    uint ViewBufferIndex;
    uint InputTextureIndex;
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

float3 GetShadingModelDisplayColor(uint ShadingModel)
{
    if (ShadingModel == SHADING_MODEL_UNLIT)
    {
        return float3(0.7f, 0.5f, 0.2f);
    }
    
    else if (ShadingModel == SHADING_MODEL_LIT)
    {
        return float3(0.3f, 0.5f, 0.7f);
    }
    
    else if (ShadingModel == SHADING_MODEL_FOLIAGE)
    {
        return float3(0.3f, 0.8f, 0.4f);
    }
    
    return 1;
}

float4 Main_PS(PixelShaderInput IN) : SV_Target
{
    ConstantBuffer<ViewBuffer> View = ResourceDescriptorHeap[BindlessResources.ViewBufferIndex];
    ConstantBuffer<StaticSamplers> StaticSamplers = ResourceDescriptorHeap[BindlessResources.StaticSamplerBufferIndex];
    
    Texture2D InputTexture = ResourceDescriptorHeap[BindlessResources.InputTextureIndex];
    SamplerState LinearSampler = ResourceDescriptorHeap[StaticSamplers.LinearSamplerIndex];
    
    float4 Sample = InputTexture.Sample(LinearSampler, IN.UV);
    float3 Result = 1;
    
#if BASECOLOR || SUBSURFACE_COLOR || PRE_TONEMAP || BLOOM || SCREEN_SPACE_REFLECTION
    Result = Sample.rgb;
#elif METALLIC || DEPTH || SCREEN_SPACE_AO
    Result = Sample.rrr;
#elif ROUGHNESS
    Result = Sample.ggg;
#elif MATERIAL_AO
    Result = Sample.bbb;
#elif SHADING_MODEL
    Result = GetShadingModelDisplayColor(FloatToUint8(Sample.a));
#elif WORLD_NORMAL
    Result = DecodeNormal(Sample.rg);
#elif LINEAR_DEPTH
    Result = ConvertFromDeviceZ(Sample.r, View.InvDeviceZToWorldZTransform).rrr / 1000.0f;
#endif
    
    return float4(Result, 1);
}