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
    VECTOR(SandColor, SandColor)
    VECTOR(SandNormalDirection, SandNormalDirection)
    VECTOR(WoodColor, WoodColor)
    
    SCALAR(EnableSand, EnableSand)
    SCALAR(SandBlendBrightness, SandBlendBrightness)
    SCALAR(SandBlendPower, SandBlendPower)
    SCALAR(RotateCoord, RotateCoord)
    SCALAR(NormalIntensityWood, NormalIntensityWood)
    SCALAR(ScaleWood, ScaleWood)
    SCALAR(WoodRoughnessLow, WoodRoughnessLow)
    SCALAR(WoodRoughnessHigh, WoodRoughnessHigh)
    SCALAR(WoodDiffusePower, WoodDiffusePower)
    
    TEX2D(BaseColor, BaseColorTexture)
    TEX2D(Normal, NormalTexture)
};

//#define MAIN_PASS 1
//#define HITPROXY_PASS 1

struct VertexShaderOutput
{
    float4 Position : SV_Position;
#if MAIN_PASS
    float3x3 TBN : TBN;
    float2 UV0 : TEXCOORD0;
#endif
    
#if HITPROXY_PASS
    uint InstanceIndex : ID;
#endif
};

VertexShaderOutput Main_VS(
    VertexInput IN
#if INSTANCED && HITPROXY_PASS
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
    
    float2 UV = IN.UV0 * Parameters.ScaleWood;
    [branch]
    if(Parameters.RotateCoord)
    {
        //UV = Rotator(UV, 1, float2(0.5, 0.5), 1);
        UV = UV.yx;
    }
    
    float3 VertexNormal = IN.TBN[1];

    float3 Color = BaseColorTexture.Sample(BaseColorSampler, UV).rgb;

    float3 BaseColor = pow(Color, Parameters.WoodDiffusePower.rrr);
    BaseColor *= Parameters.WoodColor.rgb;
    
    float Roughness = lerp(Parameters.WoodRoughnessLow, Parameters.WoodRoughnessHigh, Color.r);
    float3 Masks = float3(0, Roughness, 1.0f);
    
    float3 Normal = ReconstructTextureNormal(NormalTexture.Sample(NormalSampler, UV).rg, false);

    float3 TangentNormal = Normal * float3(Parameters.NormalIntensityWood, 1, Parameters.NormalIntensityWood);
    float3 WorldNormal = normalize(mul(TangentNormal, IN.TBN));
    
    [branch]
    if (Parameters.EnableSand)
    {
        float SandMask = dot(WorldNormal, Parameters.SandNormalDirection.xyz);
        SandMask = saturate(pow(SandMask, Parameters.SandBlendPower) * Parameters.SandBlendBrightness);
        
        BaseColor = lerp(BaseColor, Parameters.SandColor.rgb, SandMask);
    }
    
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