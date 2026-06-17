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
    VECTOR(PebblesStoneColor, PebblesStoneColor)
    VECTOR(PebblesDirtColor, PebblesDirtColor)
    VECTOR(TrackColor, TrackColor)
    VECTOR(TireColor, TireColor)
    
    SCALAR(PebblesUvScale, PebblesUvScale)
    SCALAR(RoadTrackUvScale, RoadTrackUvScale)
    SCALAR(DistortionUvScale, DistortionUvScale)
    SCALAR(DistortionXScale, DistortionXScale)
    SCALAR(DistortionYScale, DistortionYScale)
    SCALAR(TrackEdgeNoiseUvScale, TrackEdgeNoiseUvScale)
    SCALAR(TrackEdgeNoiseDark, TrackEdgeNoiseDark)
    SCALAR(TrackEdgeNoiseBright, TrackEdgeNoiseBright)
    SCALAR(TrackNormalIntensity, TrackNormalIntensity)
    SCALAR(Roughness, Roughness)
    
    TEX2D(PebblesBaseColor, PebblesBaseColorTexture)
    TEX2D(PebblesNormal, PebblesNormalTexture)
    TEX2D(RoadTrackMasks, RoadTrackMasksTexture)
    TEX2D(RoadTrackNormal, RoadTrackNormalTexture)
    TEX2D(Distortion, DistortionTexture)
};

//#define MAIN_PASS 1

struct VertexShaderOutput
{
    float4 Position : SV_Position;
#if MAIN_PASS
    float3x3 TBN : TBN;
    float3 WorldPostion : POS0;
    float2 UV0 : TEXCOORD0;
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
    SamplerState PointSampler = ResourceDescriptorHeap[StaticSamplers.PointSamplerIndex];
    
    Texture2D DecalBaseColorTexture = ResourceDescriptorHeap[BindlessResources.DecalBaseColor];
    Texture2D DecalNormalTexture = ResourceDescriptorHeap[BindlessResources.DecalNormal];
    Texture2D DecalMasksTexture = ResourceDescriptorHeap[BindlessResources.DecalMasks];
    
    float2 ScreenUV = SvPositionToViewportUV(IN.Position.xy, View.InvSize);
    float4 DecalBaseColor = DecalBaseColorTexture.Sample(PointSampler, ScreenUV);
    float4 DecalNormal = DecalNormalTexture.Sample(PointSampler, ScreenUV);
    float4 DecalMasks = DecalMasksTexture.Sample(PointSampler, ScreenUV);
    
    Texture2D PebblesBaseColorTexture = ResourceDescriptorHeap[Parameters.PebblesBaseColor_Texture];
    SamplerState PebblesBaseColorSampler = ResourceDescriptorHeap[Parameters.PebblesBaseColor_Sampler];
    
    Texture2D PebblesNormalTexture = ResourceDescriptorHeap[Parameters.PebblesNormal_Texture];
    SamplerState PebblesNormalSampler = ResourceDescriptorHeap[Parameters.PebblesNormal_Sampler];
    
    Texture2D RoadTrackMasksTexture = ResourceDescriptorHeap[Parameters.RoadTrackMasks_Texture];
    SamplerState RoadTrackMasksSampler = ResourceDescriptorHeap[Parameters.RoadTrackMasks_Sampler];
    
    Texture2D RoadTrackNormalTexture = ResourceDescriptorHeap[Parameters.RoadTrackNormal_Texture];
    SamplerState RoadTrackNormalSampler = ResourceDescriptorHeap[Parameters.RoadTrackNormal_Sampler];
    
    Texture2D DistortionTexture = ResourceDescriptorHeap[Parameters.Distortion_Texture];
    SamplerState DistortionSampler = ResourceDescriptorHeap[Parameters.Distortion_Sampler];
    
    float2 UV0 = IN.UV0;;
    
    float3 VertexNormal = IN.TBN[1];
    
    //float2 PebblesUV = UV0 * Parameters.PebblesUvScale;
    float2 PebblesUV = IN.WorldPostion.xz * Parameters.PebblesUvScale;
    float4 PebblesTexture = PebblesBaseColorTexture.Sample(PebblesBaseColorSampler, PebblesUV);
    float StoneMask = PebblesTexture.a;
    float3 PebblesColorTint = lerp(Parameters.PebblesDirtColor.rgb, Parameters.PebblesStoneColor.rgb, StoneMask);
    float3 PebblesColor = PebblesTexture.rgb * PebblesColorTint;
    
    float2 UvDistortion = DistortionTexture.Sample(DistortionSampler, IN.WorldPostion.xz / Parameters.DistortionUvScale).rg;
    UvDistortion *= float2(Parameters.DistortionXScale, Parameters.DistortionYScale);
    float2 RoadTrackUv = UV0 * Parameters.RoadTrackUvScale + UvDistortion;

    float TireTrackMask = RoadTrackMasksTexture.Sample(RoadTrackMasksSampler, RoadTrackUv).r;
    float3 TrackColor = lerp(Parameters.TrackColor.rgb, Parameters.TireColor.rgb, TireTrackMask);
    
    float TrackEdgeNoise = RoadTrackMasksTexture.Sample(RoadTrackMasksSampler, IN.WorldPostion.xz / Parameters.TrackEdgeNoiseUvScale).g;
    TrackEdgeNoise = saturate(lerp(Parameters.TrackEdgeNoiseDark, Parameters.TrackEdgeNoiseBright, TrackEdgeNoise));
    
    float TrackEdgeMask = RoadTrackMasksTexture.Sample(RoadTrackMasksSampler, UV0).b;
    TrackEdgeMask = saturate(TrackEdgeMask * TrackEdgeNoise * (1 - StoneMask));
    
    float3 BaseColor = lerp(PebblesColor, TrackColor, TrackEdgeMask);
    
    float3 TrackNormal = ReconstructTextureNormal(RoadTrackNormalTexture.Sample(RoadTrackNormalSampler, RoadTrackUv).rg, true);
    TrackNormal *= Parameters.TrackNormalIntensity;
    float3 PebblesNormal = ReconstructTextureNormal(PebblesNormalTexture.Sample(PebblesNormalSampler, PebblesUV).rg, true);
    float3 TangentNormal = lerp(PebblesNormal, TrackNormal, TrackEdgeMask);
    float3 WorldNormal = normalize(mul(TangentNormal, IN.TBN));
    WorldNormal = lerp(DecalNormal.xyz, WorldNormal, DecalNormal.w);
    
    float Roughness = Parameters.Roughness + (1 - TireTrackMask);
    
    float3 Masks = float3(0, Roughness, 1.0f);
    
    OUT.ColorDeferred = float4(0.0, 0.0, 0.0, 1);
    OUT.BaseColor = float4(BaseColor, 1);
    OUT.WorldNormal = EncodeNormal(WorldNormal);
    OUT.Masks = float4(Masks, Uint8ToFloat(SHADING_MODEL_LIT));
    
    OUT.BaseColor.xyz = lerp(DecalBaseColor.xyz, OUT.BaseColor.xyz, DecalBaseColor.w);
    OUT.Masks.xyz = lerp(DecalMasks.xyz, OUT.Masks.xyz, DecalMasks.w);
    
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