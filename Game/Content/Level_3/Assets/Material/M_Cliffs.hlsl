#include "Common.hlsl"

// DOMAIN_SURFACE
// BLEND_OPAQUE
// SHADING_LIT

// SUPPORT_STATICMESH
// SUPPORT_INSTANCED

// SUPPORT_MAIN_PASS
// SUPPORT_PRE_PASS
// SUPPORT_HIT_PROXY_PASS
// SUPPORT_EDITOR_SELECTION_PASS
// SUPPORT_SHADOW_PASS

ConstantBuffer<StandardResources> BindlessResources : register(b0);

struct ParametersBuffers
{
    VECTOR(Color1, Color1)
    VECTOR(Color2, Color2)
    VECTOR(SandColor, SandColor)
    
    SCALAR(AoPower, AoPower)
    SCALAR(DetailScale, DetailScale)
    SCALAR(SandScale, SandScale)
    SCALAR(MaskContrast, MaskContrast)
    SCALAR(MaskBrightness, MaskBrightness)
    SCALAR(RoughnessLow, RoughnessLow)
    SCALAR(RoughnessHigh, RoughnessHigh)
    SCALAR(RoughnessSand, RoughnessSand)
    
    TEX2D(CliffMasks, CliffMasksTexture)
    TEX2D(CliffNormal, CliffNormalTexture)
    TEX2D(RockCliffBaseColor, RockCliffBaseColorTexture)
    TEX2D(RockCliffNormal, RockCliffNormalTexture)
    TEX2D(SandBaseColor, SandBaseColor)
    TEX2D(SandNormal, NormalColor)
};

//#define MAIN_PASS 1

struct VertexShaderOutput
{
    float4 Position : SV_Position;
#if MAIN_PASS
    float3x3 TBN : TBN;
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
    
    Texture2D CliffMasksTexture = ResourceDescriptorHeap[Parameters.CliffMasks_Texture];
    SamplerState CliffMasksSampler = ResourceDescriptorHeap[Parameters.CliffMasks_Sampler];
    
    Texture2D CliffNormalTexture = ResourceDescriptorHeap[Parameters.CliffNormal_Texture];
    SamplerState CliffNormalSampler = ResourceDescriptorHeap[Parameters.CliffNormal_Sampler];
    
    Texture2D RockCliffBaseColorTexture = ResourceDescriptorHeap[Parameters.RockCliffBaseColor_Texture];
    SamplerState RockCliffBaseColorSampler = ResourceDescriptorHeap[Parameters.RockCliffBaseColor_Sampler];
    
    Texture2D RockCliffNormalTexture = ResourceDescriptorHeap[Parameters.RockCliffNormal_Texture];
    SamplerState RockCliffNormalSampler = ResourceDescriptorHeap[Parameters.RockCliffNormal_Sampler];
    
    Texture2D SandBaseColorTexture = ResourceDescriptorHeap[Parameters.SandBaseColor_Texture];
    SamplerState SandBaseColorSampler = ResourceDescriptorHeap[Parameters.SandBaseColor_Sampler];
    
    Texture2D SandNormalTexture = ResourceDescriptorHeap[Parameters.SandNormal_Texture];
    SamplerState SandNormalSampler = ResourceDescriptorHeap[Parameters.SandNormal_Sampler];

    float2 UV0 = IN.UV0;;
    float2 DetailUv = UV0 * Parameters.DetailScale;
    float2 SandUv = UV0 * Parameters.SandScale;
    float3 VertexNormal = IN.TBN[1];
    
    
    float4 CliffMasks = CliffMasksTexture.Sample(CliffMasksSampler, UV0);
    float4 CliffNormal = CliffNormalTexture.Sample(CliffNormalSampler, UV0);
    float4 RockCliffBaseColor = RockCliffBaseColorTexture.Sample(RockCliffBaseColorSampler, DetailUv);
    float4 RockCliffNormal = RockCliffNormalTexture.Sample(RockCliffNormalSampler, DetailUv);
    float4 SandBaseColor = SandBaseColorTexture.Sample(SandBaseColorSampler, SandUv);
    float4 SandNormal = SandNormalTexture.Sample(SandNormalSampler, SandUv);
    
    float SandNormalMask = saturate(pow(dot(VertexNormal, float3(0, 1, 0)), Parameters.MaskContrast) * Parameters.MaskBrightness);
    float3 DetailNormal = ReconstructTextureNormal(lerp(RockCliffNormal.rg, SandNormal.rg, SandNormalMask), true);
    float3 MeshNormal = ReconstructTextureNormal(CliffNormal.rg, true);
    float3 TangentNormal = BlendAngleCorrectedNormals(MeshNormal, DetailNormal);
    float3 WorldNormal = normalize(mul(TangentNormal, IN.TBN));
    
    float SandMask = saturate(pow(dot(WorldNormal, float3(0, 1, 0)), Parameters.MaskContrast) * Parameters.MaskBrightness);


    float3 BaseColor = lerp(Parameters.Color1.rgb, Parameters.Color2.rgb, pow(CliffMasks.r, Parameters.AoPower)) * CliffMasks.r;
    BaseColor *= RockCliffBaseColor.rgb;
    BaseColor = lerp(BaseColor, SandBaseColor.rgb * Parameters.SandColor.rgb, SandMask);
    
    float Roughness = lerp(Parameters.RoughnessHigh, Parameters.RoughnessLow, RockCliffBaseColor.b);
    Roughness = lerp(Roughness, Parameters.RoughnessSand, SandMask);
    
    float3 Masks = float3(0, Roughness, 1.0f);
    
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