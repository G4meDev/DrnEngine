#include "Common.hlsl"

// DOMAIN_SURFACE
// BLEND_OPAQUE
// SHADING_UNLIT

// SUPPORT_STATICMESH

// SUPPORT_MAIN_PASS
// SUPPORT_PRE_PASS
// SUPPORT_HIT_PROXY_PASS
// SUPPORT_EDITOR_SELECTION_PASS

ConstantBuffer<StandardResources> BindlessResources : register(b0);

struct ParametersBuffers
{
    VECTOR(TintColor, TintColor)
    
    SCALAR(Exposure, Exposure)
    SCALAR(Intensity, Intensity)
    SCALAR(DistortionTile, DistortionTile)
    SCALAR(DistortionAmount, DistortionAmount)
    SCALAR(DistortionSpeed, DistortionSpeed)

    TEX2D(Sky, SkyTexture)
    TEX2D(Distortion, DistortionTexture)
};

struct VertexShaderOutput
{
    float2 UV1 : TEXCOORD1;
    float2 UV2 : TEXCOORD2;
    float4 Position : SV_Position;
};

VertexShaderOutput Main_VS(VertexInputStaticMesh IN)
{
    VertexShaderOutput OUT;

    ConstantBuffer<ViewBuffer> View = ResourceDescriptorHeap[BindlessResources.ViewIndex];
    ConstantBuffer<PrimitiveBuffer> Primitive = ResourceDescriptorHeap[BindlessResources.PrimitiveIndex];
    
    float4 WorldPosition = mul(Primitive.LocalToWorld, float4(IN.Position, 1.0f));
    
    OUT.Position = mul(View.WorldToProjection, WorldPosition);
    OUT.UV1 = IN.UV1;
    OUT.UV2 = IN.UV3;
    
    return OUT;
}

//// -------------------------------------------------------------------------------------

struct PixelShaderInput
{
#if SHADOW_PASS
    float4 Position : SV_Position;
#else
    float2 UV1 : TEXCOORD1;
    float2 UV2 : TEXCOORD2;
#endif
};

struct PixelShaderOutput
{
#if MAIN_PASS
    float4 ColorDeferred : SV_TARGET0;
    float4 BaseColor : SV_TARGET1;
    float4 WorldNormal : SV_TARGET2;
    float4 Masks : SV_TARGET3;
    float4 MasksB : SV_TARGET4;
#elif HITPROXY_PASS
    uint4 Guid;
#elif EDITOR_PRIMITIVE_PASS
    float4 Color;
#elif SHADOW_PASS
#endif
};

//#define MAIN_PASS 1

PixelShaderOutput Main_PS(PixelShaderInput IN) : SV_Target
{
    PixelShaderOutput OUT;
 
#if MAIN_PASS

    ConstantBuffer<ViewBuffer> View = ResourceDescriptorHeap[BindlessResources.ViewIndex];
    ConstantBuffer<ParametersBuffers> Parameters = ResourceDescriptorHeap[BindlessResources.ParametersBufferIndex];

    Texture2D SkyTexture = ResourceDescriptorHeap[Parameters.Sky_Texture];
    SamplerState SkySampler = ResourceDescriptorHeap[Parameters.Sky_Sampler];
    
    Texture2D DistortionTexture = ResourceDescriptorHeap[Parameters.Distortion_Texture];
    SamplerState DistortionSampler = ResourceDescriptorHeap[Parameters.Distortion_Sampler];

    float2 UV = Parameters.DistortionTile * IN.UV2 + View.GameTime * Parameters.DistortionSpeed;
    float2 DistortedUV = (DistortionTexture.Sample(DistortionSampler, UV).rg - 0.5f) * Parameters.DistortionAmount + IN.UV1;
    
    float3 BaseColor = SkyTexture.Sample(SkySampler, DistortedUV).xyz;
    
    float3 HDR = Desaturation(BaseColor, float3(0.3f, 0.59f, 0.11f), 1.0f);
    HDR = pow(HDR, Parameters.Exposure) * Parameters.Intensity + 1;
    BaseColor *= HDR;
    
    OUT.ColorDeferred = float4(BaseColor, 1);
    OUT.BaseColor = 0;
    OUT.WorldNormal = 0;
    OUT.Masks = 0;
    OUT.Masks.a = Uint8ToFloat(SHADING_MODEL_UNLIT);
    
#elif HITPROXY_PASS
    ConstantBuffer<PrimitiveBuffer> P = ResourceDescriptorHeap[BindlessResources.PrimitiveIndex];
    OUT.Guid = P.Guid;
#endif
    
    return OUT;
}