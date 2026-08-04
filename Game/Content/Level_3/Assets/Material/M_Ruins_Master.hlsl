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
    VECTOR(PaintedColor, PaintedColor)
    VECTOR(StoneColorDark, StoneColorDark)
    VECTOR(StoneColorLight, StoneColorLight)
    
    SCALAR(UsePaintTexture, UsePaintTexture)
    SCALAR(TextureScale, TextureScale)
    SCALAR(Roughness, Roughness)
    SCALAR(PaintedRoughness, PaintedRoughness)
    SCALAR(PaintModMin, PaintModMin)
    SCALAR(PaintModMax, PaintModMax)
    
    TEX2D(BaseColor, BaseColorTexture)
    TEX2D(Normal, NormalTexture)
    TEX2D(PaintMod, PaintModTexture)
    TEX2D(PaintColor, PaintColorTexture)
    TEX2D(PaintNormal, PaintNormalTexture)
};

//#define MAIN_PASS 1
//#define HITPROXY_PASS 1

struct VertexShaderOutput
{
    float4 Position : SV_Position;
#if MAIN_PASS
    float3x3 TBN : TBN;
    float2 UV0 : TEXCOORD0;
    float3 VertexColor : COLOR;
#endif
    
#if HITPROXY_PASS
    uint InstanceIndex : ID;
#endif
};

VertexShaderOutput Main_VS(
    VertexInput IN
#if INSTANCED
    , uint InstanceIndex : SV_InstanceID
#endif
)
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
    OUT.VertexColor = IN.Color;
#endif
    
#if HITPROXY_PASS
#if INSTANCED
    OUT.InstanceIndex = InstanceIndex;
#else
    OUT.InstanceIndex = 0;
#endif
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
    float3 VertexColor : COLOR;
#endif
    
#if HITPROXY_PASS
    uint InstanceIndex : ID;
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
    
    Texture2D BaseColorTexture = ResourceDescriptorHeap[Parameters.BaseColor_Texture];
    SamplerState BaseColorSampler = ResourceDescriptorHeap[Parameters.BaseColor_Sampler];

    Texture2D NormalTexture = ResourceDescriptorHeap[Parameters.Normal_Texture];
    SamplerState NormalSampler = ResourceDescriptorHeap[Parameters.Normal_Sampler];
    
    Texture2D PaintModTexture = ResourceDescriptorHeap[Parameters.PaintMod_Texture];
    SamplerState PaintModSampler = ResourceDescriptorHeap[Parameters.PaintMod_Sampler];
    
    Texture2D PaintColorTexture = ResourceDescriptorHeap[Parameters.PaintColor_Texture];
    SamplerState PaintColorSampler = ResourceDescriptorHeap[Parameters.PaintColor_Sampler];
    
    Texture2D PaintNormalTexture = ResourceDescriptorHeap[Parameters.PaintNormal_Texture];
    SamplerState PaintNormalSampler = ResourceDescriptorHeap[Parameters.PaintNormal_Sampler];
    
    float2 UV = IN.UV0 * Parameters.TextureScale;
    
    float3 VertexNormal = IN.TBN[1];

    float3 Color = BaseColorTexture.Sample(BaseColorSampler, UV).rgb;
    
    float3 PaintColor = PaintColorTexture.Sample(PaintColorSampler, UV).rgb;
    PaintColor *= Parameters.PaintedColor.rgb;
    
    float3 StoneColor = lerp(Parameters.StoneColorDark.rgb, Parameters.StoneColorLight.rgb, saturate(lerp(-1, 2, Color.r)).xxx) * Color;
    
    float StoneMask = IN.VertexColor.r;
    
    [branch]
    if(Parameters.UsePaintTexture > 0)
    {
        StoneMask *= 2;
        StoneMask += lerp(Parameters.PaintModMin, Parameters.PaintModMax, PaintModTexture.Sample(PaintModSampler, UV).r);
    }
    StoneMask = saturate(StoneMask);
    
    float3 BaseColor = lerp(PaintColor, StoneColor, StoneMask.rrr);
    float Roughness = lerp(Parameters.PaintedRoughness, Parameters.Roughness, StoneMask);

    float3 Masks = float3(0, Roughness, 1.0f);
    
    float3 PaintNormal = ReconstructTextureNormal(PaintNormalTexture.Sample(PaintNormalSampler, UV).rg, false);
    float3 Normal = ReconstructTextureNormal(NormalTexture.Sample(NormalSampler, UV).rg, false);

    float3 TangentNormal = lerp(PaintNormal, Normal, StoneMask);
    float3 WorldNormal = normalize(mul(TangentNormal, IN.TBN));
    
    OUT.ColorDeferred = float4(0.0, 0.0, 0.0, 1);
    OUT.BaseColor = float4(BaseColor, 1);
    OUT.WorldNormal = EncodeNormal(WorldNormal);
    OUT.Masks = float4(Masks, Uint8ToFloat(SHADING_MODEL_LIT));
    
#elif HITPROXY_PASS
    ConstantBuffer<PrimitiveBuffer> P = ResourceDescriptorHeap[BindlessResources.PrimitiveIndex];
    OUT.Guid = P.Guid;
    OUT.Guid[2] = IN.InstanceIndex;
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