#include "Common.hlsl"

// DOMAIN_SURFACE
// BLEND_OPAQUE
// SHADING_LIT

// SUPPORT_STATICMESH

// SUPPORT_MAIN_PASS
// SUPPORT_PRE_PASS
// SUPPORT_HIT_PROXY_PASS
// SUPPORT_EDITOR_SELECTION_PASS
// SUPPORT_SHADOW_PASS

ConstantBuffer<StandardResources> BindlessResources : register(b0);

struct ParametersBuffers
{
    VECTOR(SandColor1, SandColor1)
    VECTOR(SandColor2, SandColor2)
    VECTOR(SandColor3, SandColor3)
    
    SCALAR(SandZBlend, SandZBlend)
    SCALAR(SandZBlendOffset, SandZBlendOffset)
    SCALAR(DunesBlendPower, DunesBlendPower)
    SCALAR(DunesBlendBrighness, DunesBlendBrighness)
    SCALAR(SandAlbedoUvScale, SandAlbedoUvScale)
    SCALAR(SandNormalUvScale, SandNormalUvScale)
    SCALAR(SandNormalIntensity, SandNormalIntensity)
    SCALAR(SandRoughness, SandRoughness)
    
    TEX2D(SandBaseColor, SandBaseColorTexture)
    TEX2D(SandNormal, SandNormalTexture)
    TEX2D(SandMasks, SandMasksTexture)
    TEX2D(SandDunesNormal, SandDunesNormalTexture)
};

//#define MAIN_PASS 1

struct VertexShaderOutput
{
    float4 Position : SV_Position;
#if MAIN_PASS
    float3x3 TBN : TBN;
    float3 WorldPostion : POS0;
#endif
};

VertexShaderOutput Main_VS(VertexInput IN)
{
    VertexShaderOutput OUT;

    ConstantBuffer<ViewBuffer> View = ResourceDescriptorHeap[BindlessResources.ViewIndex];
    ConstantBuffer<PrimitiveBuffer> Primitive = ResourceDescriptorHeap[BindlessResources.PrimitiveIndex];
    ConstantBuffer<ParametersBuffers> Parameters = ResourceDescriptorHeap[BindlessResources.ParametersBufferIndex];

    matrix LocalToWorld;
#if STATICMESH
    LocalToWorld = Primitive.LocalToWorld;
#elif INSTANCED
    LocalToWorld = GetLocalToWorld(IN);
#endif
    
    float4 WorldPosition = mul(LocalToWorld, float4(IN.Position, 1.0f));
    
#if SHADOW_PASS_POINTLIGHT
    OUT.Position = WorldPosition;
#elif SHADOW_PASS_SPOTLIGHT
    ConstantBuffer<ShadowDepth> ShadowBuffer = ResourceDescriptorHeap[BindlessResources.ShadowDepthBuffer];
    OUT.Position = mul(ShadowBuffer.WorldToProjectionMatrix, WorldPosition);
#else
    OUT.Position = mul(View.WorldToProjection, WorldPosition);
#endif
    
#if MAIN_PASS
    float3 WorldNormal = normalize(mul((float3x3)LocalToWorld, IN.Normal));
    float3 WorldTangent = normalize(mul((float3x3)LocalToWorld, IN.Tangent));
    OUT.TBN = GetTBN(WorldNormal, WorldTangent);
    
    OUT.WorldPostion = WorldPosition.xyz;
#endif
    
    return OUT;
}

//// -------------------------------------------------------------------------------------

struct PixelShaderInput
{
    float4 Position : SV_Position;
#if MAIN_PASS
    float3x3 TBN : TBN;
    float3 WorldPostion : POS0;
#endif
};

struct PixelShaderOutput
{
#if MAIN_PASS
    float4 ColorDeferred : SV_TARGET0;
    float4 BaseColor : SV_TARGET1;
    float2 WorldNormal : SV_TARGET2;
    float4 Masks : SV_TARGET3;
    float4 MasksB : SV_TARGET4;
#elif HITPROXY_PASS
    uint4 Guid;
#elif EDITOR_PRIMITIVE_PASS
    float4 Color;
#elif SHADOW_PASS
#endif
};

PixelShaderOutput Main_PS(PixelShaderInput IN) : SV_Target
{
    PixelShaderOutput OUT;
 
#if MAIN_PASS

    ConstantBuffer<ParametersBuffers> Parameters = ResourceDescriptorHeap[BindlessResources.ParametersBufferIndex];

    ConstantBuffer<ViewBuffer> View = ResourceDescriptorHeap[BindlessResources.ViewIndex];
    ConstantBuffer<StaticSamplers> StaticSamplers = ResourceDescriptorHeap[BindlessResources.StaticSamplerBufferIndex];
    SamplerState LinearSampler = ResourceDescriptorHeap[StaticSamplers.LinearSamplerIndex];
    
    Texture2D SandBaseColorTexture = ResourceDescriptorHeap[Parameters.SandBaseColor_Texture];
    SamplerState SandBaseColorSampler = ResourceDescriptorHeap[Parameters.SandBaseColor_Sampler];
    
    Texture2D SandNormalTexture = ResourceDescriptorHeap[Parameters.SandNormal_Texture];
    SamplerState SandNormalSampler = ResourceDescriptorHeap[Parameters.SandNormal_Sampler];
    
    Texture2D SandMasksTexture = ResourceDescriptorHeap[Parameters.SandMasks_Texture];
    SamplerState SandMasksSampler = ResourceDescriptorHeap[Parameters.SandMasks_Sampler];
    
    Texture2D SandDunesNormalTexture = ResourceDescriptorHeap[Parameters.SandDunesNormal_Texture];
    SamplerState SandDunesNormalSampler = ResourceDescriptorHeap[Parameters.SandDunesNormal_Sampler];
    
    float3 UpVector = float3(0, 1, 0);
    float3 VertexNormal = IN.TBN[1];
    
    float DunesAngleMask = saturate(pow(dot(VertexNormal, UpVector), Parameters.DunesBlendPower) * Parameters.DunesBlendBrighness);
    
    float ZBlend = saturate(IN.WorldPostion.y / Parameters.SandZBlend + Parameters.SandZBlendOffset);

    float3 SandColor = lerp(Parameters.SandColor1.rgb, Parameters.SandColor2.rgb, ZBlend);
    SandColor = lerp(Parameters.SandColor3.rgb, SandColor, DunesAngleMask);
    SandColor *= SandBaseColorTexture.Sample(SandBaseColorSampler, IN.WorldPostion.xz * Parameters.SandAlbedoUvScale).rgb;
    float2 SandNormalUv = IN.WorldPostion.xz * Parameters.SandNormalUvScale;
    SandColor *= SandNormalTexture.Sample(SandNormalSampler, SandNormalUv).r;
    
    float3 SandNormal = ReconstructTextureNormal(SandNormalTexture.Sample(SandNormalSampler, SandNormalUv).rg, true);
    SandNormal.xz *= Parameters.SandNormalIntensity;
    SandNormal.y = 1;
    
    float SandRoughness = Parameters.SandRoughness;

// --------------------------------------------
    
    float3 TangentNormal = SandNormal;
    float3 WorldNormal = normalize(mul(TangentNormal, IN.TBN));
    
    float3 BaseColor = SandColor;
    float3 Masks = float3(0, SandRoughness, 1);
    
    OUT.ColorDeferred = float4(0.0, 0.0, 0.0, 1);
    OUT.BaseColor = float4(BaseColor, 1);
    OUT.WorldNormal = EncodeNormal(WorldNormal);
    OUT.Masks = float4(Masks, Uint8ToFloat(SHADING_MODEL_LIT));
    
#elif HITPROXY_PASS
    ConstantBuffer<PrimitiveBuffer> P = ResourceDescriptorHeap[BindlessResources.PrimitiveIndex];
    OUT.Guid = P.Guid;
#endif
    
    return OUT;
}

// -------------------------------------------------------------------------------------

struct GeometeryShaderOutput
{
    float4 Position : SV_Position;
    uint TargetIndex : SV_RenderTargetArrayIndex;
};

#if SHADOW_PASS_POINTLIGHT

[maxvertexcount(18)]
void PointLightShadow_GS(triangle VertexShaderOutput input[3], inout TriangleStream<GeometeryShaderOutput> OutputStream)
{
    ConstantBuffer<ShadowDepth> ShadowDepthBuffer = ResourceDescriptorHeap[BindlessResources.ShadowDepthBuffer];
    
    [unroll]
    for (int CubeFaceIndex = 0; CubeFaceIndex < 6; CubeFaceIndex++)
    {
        [unroll]
		for (int VertexIndex = 0; VertexIndex < 3; VertexIndex++)
		{
            GeometeryShaderOutput OUT;
            OUT.TargetIndex = CubeFaceIndex;

            float3 WorldPosition = input[VertexIndex].Position.xyz;
            OUT.Position = mul(ShadowDepthBuffer.WorldToProjectionMatrices[CubeFaceIndex], float4(WorldPosition, 1.0f));
            OutputStream.Append(OUT);
        }
		OutputStream.RestartStrip();
    }
}

#endif