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
    VECTOR(DustColor, DustColor)
    VECTOR(PebblesDirtColor, PebblesDirtColor)
    VECTOR(PebblesStoneColor, PebblesStoneColor)
    VECTOR(RocksColor1, RocksColor1)
    VECTOR(RocksColor2, RocksColor2)
    
    SCALAR(SandZBlend, SandZBlend)
    SCALAR(SandZBlendOffset, SandZBlendOffset)
    SCALAR(DunesNormalScale, DunesNormalScale)
    SCALAR(DunesNormalIntensity, DunesNormalIntensity)
    SCALAR(DunesBlendPower, DunesBlendPower)
    SCALAR(DunesBlendBrighness, DunesBlendBrighness)
    SCALAR(SandAlbedoUvScale, SandAlbedoUvScale)
    SCALAR(SandNormalUvScale, SandNormalUvScale)
    SCALAR(SandNormalIntensity, SandNormalIntensity)
    SCALAR(SandRoughness, SandRoughness)
    SCALAR(PebblesScale, PebblesScale)
    
    SCALAR(DustDistortionSize, DustDistortionSize)
    SCALAR(DustDistortionSpeed, DustDistortionSpeed)
    SCALAR(DustDistortionStrength, DustDistortionStrength)
    SCALAR(DustSpeed, DustSpeed)
    SCALAR(DustScale, DustScale)
    SCALAR(DustFadeDistance, DustFadeDistance)
    SCALAR(DustFadeOffset, DustFadeOffset)
    SCALAR(RocksScale, RocksScale)
    SCALAR(TilingRocksScale, TilingRocksScale)
    SCALAR(RocksAoPower, RocksAoPower)
    SCALAR(RocksRoughnessLow, RocksRoughnessLow)
    SCALAR(RocksRoughnessHigh, RocksRoughnessHigh)
    SCALAR(RocksDetailNormalIntensity, RocksDetailNormalIntensity)
    SCALAR(RocksMaskContrast, RocksMaskContrast)
    SCALAR(RocksMaskBrightness, RocksMaskBrightness)
    SCALAR(RocksLerpContrast, RocksLerpContrast)
    SCALAR(RocksTintMaskContrast, RocksTintMaskContrast)
    SCALAR(RocksTintMaskBrightness, RocksTintMaskBrightness)
    
    TEX2D(SandBaseColor, SandBaseColorTexture)
    TEX2D(SandNormal, SandNormalTexture)
    TEX2D(SandMasks, SandMasksTexture)
    TEX2D(SandDunesNormal, SandDunesNormalTexture)
    TEX2D(PebblesBaseColor, PebblesBaseColorTexture)
    TEX2D(PebblesNormal, PebblesNormalTexture)
    TEX2D(RockNormal, RockNormalTexture)
    TEX2D(RockMask, RockMaskTexture)
    TEX2D(TilingRockNormal, TilingRockNormalTexture)
    TEX2D(TilingRockMask, TilingRockMaskTexture)
};

//#define MAIN_PASS 1

struct VertexShaderOutput
{
    float4 Position : SV_Position;
#if MAIN_PASS
    float3x3 TBN : TBN;
    float3 WorldPostion : POS0;
    float2 UV0 : TEXCOORD0;
    float VertexColor : MASK0;
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
    OUT.UV0 = IN.UV1;
    OUT.VertexColor = IN.Color.r;
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
    float2 UV0 : TEXCOORD0;
    float VertexColor : MASK0;
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
    
    Texture2D PebblesBaseColorTexture = ResourceDescriptorHeap[Parameters.PebblesBaseColor_Texture];
    SamplerState PebblesBaseColorSampler = ResourceDescriptorHeap[Parameters.PebblesBaseColor_Sampler];
    
    Texture2D PebblesNormalTexture = ResourceDescriptorHeap[Parameters.PebblesNormal_Texture];
    SamplerState PebblesNormalSampler = ResourceDescriptorHeap[Parameters.PebblesNormal_Sampler];
    
    Texture2D RockNormalTexture = ResourceDescriptorHeap[Parameters.RockNormal_Texture];
    SamplerState RockNormalSampler = ResourceDescriptorHeap[Parameters.RockNormal_Sampler];
    
    Texture2D RockMaskTexture = ResourceDescriptorHeap[Parameters.RockMask_Texture];
    SamplerState RockMaskSampler = ResourceDescriptorHeap[Parameters.RockMask_Sampler];
    
    Texture2D TilingRockNormalTexture = ResourceDescriptorHeap[Parameters.TilingRockNormal_Texture];
    SamplerState TilingRockNormalSampler = ResourceDescriptorHeap[Parameters.TilingRockNormal_Sampler];
    
    Texture2D TilingRockMaskTexture = ResourceDescriptorHeap[Parameters.TilingRockMask_Texture];
    SamplerState TilingRockMaskSampler = ResourceDescriptorHeap[Parameters.TilingRockMask_Sampler];
    
    float3 UpVector = float3(0, 1, 0);
    float3 VertexNormal = IN.TBN[1];
    //float PixelDepth = IN.Position.z / IN.Position.w;
    float PixelDepth = IN.Position.w;
    float2 UV0 = IN.UV0;
    //float2 UV0 = IN.WorldPostion.xz;
    float PebblesMask = IN.VertexColor.r;
    
    float DunesAngleMask = saturate(pow(dot(VertexNormal, UpVector), Parameters.DunesBlendPower) * Parameters.DunesBlendBrighness);
    
    float ZBlend = saturate(IN.WorldPostion.y / Parameters.SandZBlend + Parameters.SandZBlendOffset);

    float3 SandColor = lerp(Parameters.SandColor1.rgb, Parameters.SandColor2.rgb, ZBlend);
    SandColor = lerp(Parameters.SandColor3.rgb, SandColor, DunesAngleMask);
    SandColor *= SandBaseColorTexture.Sample(SandBaseColorSampler, UV0 * Parameters.SandAlbedoUvScale).rgb;
    float2 SandNormalUv = UV0 * Parameters.SandNormalUvScale;
    SandColor *= saturate(SandNormalTexture.Sample(SandNormalSampler, SandNormalUv).r + 0.18f);
    float SandRoughness = Parameters.SandRoughness;
    
    float2 PebblesUv = UV0 * Parameters.PebblesScale;
    float4 PebblesColorSample = PebblesBaseColorTexture.Sample(PebblesBaseColorSampler, PebblesUv);
    float PebblesStoneMask = PebblesColorSample.a;
    float3 PebblesColor = lerp(Parameters.PebblesDirtColor.rgb, Parameters.PebblesStoneColor.rgb, PebblesStoneMask);
    PebblesColor *= PebblesColorSample.rgb;
    float3 PebblesNormal = ReconstructTextureNormal(PebblesNormalTexture.Sample(PebblesNormalSampler, PebblesUv).rg, false);
    float PebblesRoughness = lerp(0.9, 1.0, PebblesStoneMask);
    
    float DustDistortionOffset = IN.WorldPostion.x / Parameters.DustDistortionSize;
    DustDistortionOffset += View.GameTime * Parameters.DustDistortionSpeed;
    DustDistortionOffset = sin(DustDistortionOffset) * Parameters.DustDistortionStrength + View.GameTime;
    DustDistortionOffset *= Parameters.DustSpeed;
    
    float2 DustUv = IN.WorldPostion.xz * Parameters.DustScale + DustDistortionOffset * float2(0.1f, 1.0f);
    float DustMask = SandMasksTexture.Sample(SandMasksSampler, DustUv).b;
    
    float3 SandNormal = ReconstructTextureNormal(SandNormalTexture.Sample(SandNormalSampler, SandNormalUv).rg, false);
    SandNormal.xz *= Parameters.SandNormalIntensity;
    SandNormal.y = 1;
    
    float3 DunesNormal = ReconstructTextureNormal(SandDunesNormalTexture.Sample(SandDunesNormalSampler, UV0 * Parameters.DunesNormalScale).rg, false);
    float3 DunesNormalScale = float3(1, 0, 1) * Parameters.DunesNormalIntensity * DunesAngleMask + float3(0, 1, 0);
    DunesNormal *= DunesNormalScale;
    
    float2 RocksUv = UV0 * Parameters.RocksScale;
    float4 RocksMaskSample = RockMaskTexture.Sample(RockMaskSampler, RocksUv);
    float4 RocksNormalSample = RockNormalTexture.Sample(RockNormalSampler, RocksUv);
    float2 TilingRocksUv = UV0 * Parameters.TilingRocksScale;
    float4 TilingRocksMaskSample = TilingRockMaskTexture.Sample(TilingRockMaskSampler, TilingRocksUv);
    float4 TilingRocksNormalSample = TilingRockNormalTexture.Sample(TilingRockNormalSampler, TilingRocksUv);
    
    float3 RocksColor = lerp(Parameters.RocksColor1.rgb, Parameters.RocksColor2.rgb, pow(TilingRocksMaskSample.r, Parameters.RocksAoPower));
    RocksColor *= RocksMaskSample.r;
    RocksColor *= TilingRocksMaskSample.r;
    
    float3 RocksNormal = BlendAngleCorrectedNormals(ReconstructTextureNormal(TilingRocksNormalSample.rg, false), 
        ReconstructTextureNormal(RocksNormalSample.rg, false) * (float3(Parameters.RocksDetailNormalIntensity, 0, Parameters.RocksDetailNormalIntensity) + float3(0, 1, 0)));
    
    float RocksRoughness = lerp(Parameters.RocksRoughnessHigh, Parameters.RocksRoughnessLow, RocksMaskSample.g);
    
    float RockAngleMask = 1 - saturate(pow(dot(VertexNormal, UpVector), Parameters.RocksMaskContrast) * Parameters.RocksMaskBrightness);
    float RockMask = RockAngleMask;
    RockMask = HeightLerp(TilingRocksMaskSample.g, RockMask, Parameters.RocksLerpContrast);
    
// --------------------------------------------
    
    float3 TangentNormal = BlendAngleCorrectedNormals(DunesNormal, SandNormal);
    TangentNormal = lerp(TangentNormal, PebblesNormal, PebblesMask);
    TangentNormal = lerp(TangentNormal, RocksNormal, RockMask);
    float3 WorldNormal = normalize(mul(TangentNormal, IN.TBN));
    
    float RockTintMask = saturate(pow(dot(WorldNormal, UpVector), Parameters.RocksTintMaskContrast) * Parameters.RocksTintMaskBrightness);
    
    float3 BaseColor = lerp(SandColor, PebblesColor, PebblesMask);
    RocksColor = lerp(BaseColor, RocksColor, RockTintMask);
    BaseColor = lerp(BaseColor, RocksColor, RockMask);
    
    DustMask *= saturate(pow(dot(WorldNormal, float3(0, 1, -1)), Parameters.DunesBlendPower) * Parameters.DunesBlendBrighness);
    DustMask *= CameraDepthFade(PixelDepth, Parameters.DustFadeOffset, Parameters.DustFadeDistance);
    BaseColor = lerp(BaseColor, Parameters.DustColor.rgb, DustMask);
    
    float Roughness = lerp(SandRoughness, PebblesRoughness, PebblesMask);
    RocksRoughness = lerp(Roughness, RocksRoughness, RockTintMask);
    Roughness = lerp(Roughness, RocksRoughness, RockMask);
    float3 Masks = float3(0, Roughness, 1);
    
    //WorldNormal = normalize(mul(RocksNormal, IN.TBN));
    
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