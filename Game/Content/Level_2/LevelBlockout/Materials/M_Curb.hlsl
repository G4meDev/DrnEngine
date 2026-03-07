#include "Common.hlsl"

// DOMAIN_SURFACE

// SUPPORT_STATICMESH

// SUPPORT_MAIN_PASS
// SUPPORT_PRE_PASS
// SUPPORT_HIT_PROXY_PASS
// SUPPORT_EDITOR_SELECTION_PASS
// SUPPORT_SHADOW_PASS

ConstantBuffer<StandardResources> BindlessResources : register(b0);

struct ParametersBuffers
{
    VECTOR(ColorTint, ColorTint)
    SCALAR(RoughnessMultiplier, RoughnessMultiplier)
    SCALAR(NormalStrength, NormalStrength)
    SCALAR(MicroUvScale, MicroUvScale)
    
    TEX2D(BaseColor, BaseColorTexture)
    TEX2D(Normal, NormalTexture)
    TEX2D(Masks, MasksTexture)
};

struct VertexShaderOutput
{
    float4 Color : COLOR;
    float3 Normal : NORMAL;
    float3x3 TBN : TBN;
    float2 UV1 : TEXCOORD1;
    float4 Position : SV_Position;
};

VertexShaderOutput Main_VS(VertexInputStaticMesh IN)
{
    VertexShaderOutput OUT;
    

#if SHADOW_PASS_POINTLIGHT
    ConstantBuffer<PrimitiveBuffer> P = ResourceDescriptorHeap[BindlessResources.PrimitiveIndex];
    OUT.Position = mul(P.LocalToWorld, float4(IN.Position, 1.0f));
    
    OUT.TBN = float3x3(IN.Position,IN.Position,IN.Position);
    OUT.Color = float4(IN.Color, 1.0f);
    OUT.Normal = IN.Position;
    OUT.UV1 = IN.UV1;
#elif SHADOW_PASS_SPOTLIGHT
    ConstantBuffer<PrimitiveBuffer> P = ResourceDescriptorHeap[BindlessResources.PrimitiveIndex];
    ConstantBuffer<ShadowDepth> ShadowBuffer = ResourceDescriptorHeap[BindlessResources.ShadowDepthBuffer];
    float3 WorldPosition = mul(P.LocalToWorld, float4(IN.Position, 1.0f)).xyz;
    OUT.Position = mul(ShadowBuffer.WorldToProjectionMatrix, float4(WorldPosition, 1));

    OUT.TBN = float3x3(IN.Position,IN.Position,IN.Position);
    OUT.Color = float4(IN.Color, 1.0f);
    OUT.Normal = IN.Position;
    OUT.UV1 = IN.UV1;
#else
    
    ConstantBuffer<PrimitiveBuffer> P = ResourceDescriptorHeap[BindlessResources.PrimitiveIndex];
    ConstantBuffer<ViewBuffer> View = ResourceDescriptorHeap[BindlessResources.ViewIndex];
    ConstantBuffer<ParametersBuffers> Parameters = ResourceDescriptorHeap[BindlessResources.ParametersBufferIndex];
    
    float3 WorldNormal = normalize(mul((float3x3) P.LocalToWorld, IN.Normal));
    float3 WorldTangent = normalize(mul((float3x3) P.LocalToWorld, IN.Tangent));
    OUT.TBN = GetTBN(WorldNormal, WorldTangent);
    //OUT.TBN = float3x3(WorldTangent, WorldNormal, VertexBiNormal);
    
    OUT.Position = mul(P.LocalToProjection, float4(IN.Position, 1.0f));
    OUT.Color = float4(IN.Color, 1.0f);
    OUT.Normal = WorldNormal;
    OUT.UV1 = IN.UV1;
    
    //float3 WorldPos = mul(P.LocalToWorld, float4(IN.Position, 1.0f)).xyz;
    //OUT.UV2 = WorldPos.xz * Parameters.MicroUvScale;
    
#endif
    
    return OUT;
}

//// -------------------------------------------------------------------------------------

struct PixelShaderInput
{
#if SHADOW_PASS
    float4 Position : SV_Position;
#else
    float4 Color : COLOR;
    float3 Normal : NORMAL;
    float3x3 TBN : TBN;
    float2 UV1 : TEXCOORD1;
    float4 Position : SV_Position;
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

//#define MAIN_PASS 1

PixelShaderOutput Main_PS(PixelShaderInput IN) : SV_Target
{
    PixelShaderOutput OUT;
 
#if MAIN_PASS

    ConstantBuffer<StaticSamplers> StaticSamplers = ResourceDescriptorHeap[BindlessResources.StaticSamplerBufferIndex];
    SamplerState LinearSampler = ResourceDescriptorHeap[StaticSamplers.LinearSamplerIndex];
    SamplerState PointSampler = ResourceDescriptorHeap[StaticSamplers.PointSamplerIndex];
    
    ConstantBuffer<ParametersBuffers> Parameters = ResourceDescriptorHeap[BindlessResources.ParametersBufferIndex];

    ConstantBuffer<ViewBuffer> View = ResourceDescriptorHeap[BindlessResources.ViewIndex];
    Texture2D DecalBaseColorTexture = ResourceDescriptorHeap[BindlessResources.DecalBaseColor];
    Texture2D DecalNormalTexture = ResourceDescriptorHeap[BindlessResources.DecalNormal];
    Texture2D DecalMasksTexture = ResourceDescriptorHeap[BindlessResources.DecalMasks];
    
    Texture2D BaseColorTexture = ResourceDescriptorHeap[Parameters.BaseColor_Texture];
    SamplerState BaseColorSampler = ResourceDescriptorHeap[Parameters.BaseColor_Sampler];
    
    Texture2D NormalTexture = ResourceDescriptorHeap[Parameters.Normal_Texture];
    SamplerState NormalSampler = ResourceDescriptorHeap[Parameters.Normal_Sampler];
    
    Texture2D MasksTexture = ResourceDescriptorHeap[Parameters.Masks_Texture];
    SamplerState MasksSampler = ResourceDescriptorHeap[Parameters.Masks_Sampler];
    
    float2 ScreenUV = SvPositionToViewportUV(IN.Position.xy, View.InvSize);
    float4 DecalBaseColor = DecalBaseColorTexture.Sample(PointSampler, ScreenUV);
    float4 DecalNormal = DecalNormalTexture.Sample(PointSampler, ScreenUV);
    float4 DecalMasks = DecalMasksTexture.Sample(PointSampler, ScreenUV);
    
    float3 BaseColor = BaseColorTexture.Sample(BaseColorSampler, IN.UV1).xyz;
    BaseColor = lerp(BaseColor, Parameters.ColorTint.xyz, Parameters.ColorTint.a);
    
    float3 Masks = MasksTexture.Sample(MasksSampler, IN.UV1).xyz;
    Masks.g *= Parameters.RoughnessMultiplier;
    
    float3 Normal = NormalTexture.Sample(NormalSampler, IN.UV1).rgb;
    Normal = ReconstructTextureNormal(Normal.xy, false);
    Normal = lerp(float3(0.0f, 1.0f, 0.0f), Normal, Parameters.NormalStrength);
    Normal = normalize(mul(Normal, IN.TBN));

    Normal = lerp(DecalNormal.xyz, Normal, DecalNormal.w);
    float2 N = EncodeNormal(Normal);
    
    
    OUT.ColorDeferred = float4(0.0, 0.0, 0.0, 1);
    OUT.BaseColor = float4(BaseColor, 1);
    OUT.WorldNormal = N;
    OUT.Masks = float4(Masks, 1.0f/255);
    
    //float2 Velocity = IN.ScreenPos.xy / IN.ScreenPos.w - IN.PrevScreenPos.xy / IN.PrevScreenPos.w;
    //Velocity = Velocity * 0.25f + 0.5f;
    //OUT.Velocity = Velocity;
    //OUT.Velocity = 0;
    
    //OUT.BaseColor.xyz = lerp(DecalBaseColor.xyz, OUT.BaseColor.xyz, DecalBaseColor.w);
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