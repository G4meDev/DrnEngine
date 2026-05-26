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
    VECTOR(ColorCleanBricks1, ColorCleanBricks1)
    VECTOR(ColorCleanBricks2, ColorCleanBricks2)
    
    SCALAR(RandomDarkening, RandomDarkening)
    SCALAR(RoughnessCleanBrickLow, RoughnessCleanBrickLow)
    SCALAR(RoughnessCleanBrickHigh, RoughnessCleanBrickHigh)
    SCALAR(DetailNormalScale2CleanBricks, DetailNormalScale2CleanBricks)
    SCALAR(DetailNormalInt2CleanBricks, DetailNormalInt2CleanBricks)
    SCALAR(DetailNormalScaleCleanBricks, DetailNormalScaleCleanBricks)
    SCALAR(DetailNormalIntCleanBricks, DetailNormalIntCleanBricks)
    
    SCALAR(MaskRWeight, MaskRWeight)
    SCALAR(MaskGWeight, MaskGWeight)
    
    TEX2D(Masks, MasksTexture)
    TEX2D(WallMasks, WallMasksTexture)
    TEX2D(BaseColor, BaseColorTexture)
    TEX2D(SandNormal, SandNormalTexture)
    TEX2D(RockNormal, RockNormalTexture)
    TEX2D(WallNormal, WallNormalTexture)
};

//#define MAIN_PASS 1

struct VertexShaderOutput
{
    float4 Position : SV_Position;
#if MAIN_PASS
    float3x3 TBN : TBN;
    float2 UV0 : TEXCOORD0;
    float2 UV1 : TEXCOORD1;
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
    OUT.UV1 = IN.UV2;
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
    float2 UV1 : TEXCOORD1;
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
    
    Texture2D MasksTexture = ResourceDescriptorHeap[Parameters.Masks_Texture];
    SamplerState MasksSampler = ResourceDescriptorHeap[Parameters.Masks_Sampler];
    
    Texture2D WallMasksTexture = ResourceDescriptorHeap[Parameters.WallMasks_Texture];
    SamplerState WallMasksSampler = ResourceDescriptorHeap[Parameters.WallMasks_Sampler];
    
    Texture2D BaseColorTexture = ResourceDescriptorHeap[Parameters.BaseColor_Texture];
    SamplerState BaseColorSampler = ResourceDescriptorHeap[Parameters.BaseColor_Sampler];
    
    Texture2D SandNormalTexture = ResourceDescriptorHeap[Parameters.SandNormal_Texture];
    SamplerState SandNormalSampler = ResourceDescriptorHeap[Parameters.SandNormal_Sampler];
    
    Texture2D RockNormalTexture = ResourceDescriptorHeap[Parameters.RockNormal_Texture];
    SamplerState RockNormalSampler = ResourceDescriptorHeap[Parameters.RockNormal_Sampler];
    
    Texture2D WallNormalTexture = ResourceDescriptorHeap[Parameters.WallNormal_Texture];
    SamplerState WallNormalSampler = ResourceDescriptorHeap[Parameters.WallNormal_Sampler];
    
    float2 UV0 = IN.UV0;
    float2 UV1 = IN.UV1;
    
    float3 VertexNormal = IN.TBN[1];

    float4 TempleMasks = MasksTexture.Sample(MasksSampler, UV1);
    float TempleMask = TempleMasks.r * Parameters.MaskRWeight + TempleMasks.g * Parameters.MaskGWeight;
    float4 WallMasks = WallMasksTexture.Sample(WallMasksSampler, UV0);
    
    float3 BaseColor = lerp(Parameters.ColorCleanBricks1.rgb, Parameters.ColorCleanBricks2.rgb, TempleMask);
    BaseColor *= BaseColorTexture.Sample(BaseColorSampler, UV0).rgb;
    float3 CachedColor = BaseColor;
    BaseColor *= saturate(Desaturation(WallMasks.rgb, DefaultLuminanceFactors, 1.0f) + Parameters.RandomDarkening);
    
    float Roughness = lerp(Parameters.RoughnessCleanBrickHigh, Parameters.RoughnessCleanBrickLow, Desaturation(CachedColor, DefaultLuminanceFactors, 1.0f).r);
    float3 Masks = float3(0, Roughness, 1.0f);
    
    float3 WallNormal = ReconstructTextureNormal(WallNormalTexture.Sample(WallNormalSampler, UV0).rg, true);
    
    float2 RockUV = lerp(float2(0, 0), float2(0.3566, 0.23426), WallMasks.r);
    RockUV = lerp(RockUV, float2(1.244623, 2.6732), WallMasks.g);
    RockUV = lerp(RockUV, float2(2.146236, 0.6854), WallMasks.b);
    RockUV += UV0;
    RockUV *= Parameters.DetailNormalScale2CleanBricks;
    
    float3 SandNormal = ReconstructTextureNormal(SandNormalTexture.Sample(SandNormalSampler, UV0 * Parameters.DetailNormalScaleCleanBricks).rg, true);
    SandNormal *= float3(-Parameters.DetailNormalIntCleanBricks, 1.0f, -Parameters.DetailNormalIntCleanBricks);
    
    float3 RockNormal = ReconstructTextureNormal(RockNormalTexture.Sample(RockNormalSampler, RockUV).rg, true);
    RockNormal *= float3(Parameters.DetailNormalInt2CleanBricks, 1.0f, Parameters.DetailNormalInt2CleanBricks);
    
    float3 TangentNormal = BlendAngleCorrectedNormals(WallNormal, RockNormal);
    TangentNormal = BlendAngleCorrectedNormals(TangentNormal, SandNormal);
    float3 WorldNormal = normalize(mul(TangentNormal, IN.TBN));
    
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