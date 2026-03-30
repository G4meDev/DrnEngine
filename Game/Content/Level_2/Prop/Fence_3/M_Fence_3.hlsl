//#define INSTANCED 1
//#define STATICMESH 1
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
    VECTOR(ColorTint, ColorTint)
    SCALAR(RoughnessMultiplier, RoughnessMultiplier)
    SCALAR(NormalStrength, NormalStrength)
    SCALAR(GrungeStrength, GrungeStrength)
    
    TEX2D(BaseColor, BaseColorTexture)
    TEX2D(Normal, NormalTexture)
    TEX2D(Masks, MasksTexture)
    TEX2D(Grunge, GrungeTexture)
};

struct VertexShaderOutput
{
    float4 Color : COLOR;
    float3 Normal : NORMAL;
    float3x3 TBN : TBN;
    float2 UV1 : TEXCOORD1;
    float Rand : DATA1;
    float4 Position : SV_Position;
};

//#define SHADOW_PASS_POINTLIGHT 1
//#define SHADOW_PASS_SPOTLIGHT 1

VertexShaderOutput Main_VS(
    VertexInput IN
#if INSTANCED
    , uint InstanceIndex : SV_InstanceID
#endif
)
{
    VertexShaderOutput OUT;
    
    ConstantBuffer<ViewBuffer> View = ResourceDescriptorHeap[BindlessResources.ViewIndex];
    ConstantBuffer<PrimitiveBuffer> P = ResourceDescriptorHeap[BindlessResources.PrimitiveIndex];

    matrix LocalToWorld;
    
#if STATICMESH
    LocalToWorld = P.LocalToWorld;
    OUT.Rand = 0.0f;
   
#elif INSTANCED
    LocalToWorld = GetLocalToWorld(IN);
    OUT.Rand = IN.OriginRandom.w;
#endif
    
    float4 WorldPosition = mul(LocalToWorld, float4(IN.Position, 1));
    
#if SHADOW_PASS_POINTLIGHT
    OUT.Position = WorldPosition;
    
    OUT.TBN = float3x3(IN.Position,IN.Position,IN.Position);
    OUT.Color = float4(IN.Color, 1.0f);
    OUT.Normal = IN.Position;
    OUT.UV1 = IN.UV1;
#elif SHADOW_PASS_SPOTLIGHT
    ConstantBuffer<ShadowDepth> ShadowBuffer = ResourceDescriptorHeap[BindlessResources.ShadowDepthBuffer];
    OUT.Position = mul(ShadowBuffer.WorldToProjectionMatrix, WorldPosition);

    OUT.TBN = float3x3(IN.Position,IN.Position,IN.Position);
    OUT.Color = float4(IN.Color, 1.0f);
    OUT.Normal = IN.Position;
    OUT.UV1 = IN.UV1;
#else
    
    float3 WorldNormal = normalize(mul((float3x3)LocalToWorld, IN.Normal));
    float3 WorldTangent = normalize(mul((float3x3)LocalToWorld, IN.Tangent));
    OUT.TBN = GetTBN(WorldNormal, WorldTangent);
    
    OUT.Position = mul(View.WorldToProjection, WorldPosition);
    OUT.Color = float4(IN.Color, 1.0f);
    OUT.Normal = WorldNormal;
    OUT.UV1 = IN.UV1;
    
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
    float Rand : DATA1;
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
    
    Texture2D GrungeTexture = ResourceDescriptorHeap[Parameters.Grunge_Texture];
    SamplerState GrungeSampler = ResourceDescriptorHeap[Parameters.Grunge_Sampler];
    
    float2 ScreenUV = SvPositionToViewportUV(IN.Position.xy, View.InvSize);
    float4 DecalBaseColor = DecalBaseColorTexture.Sample(PointSampler, ScreenUV);
    float4 DecalNormal = DecalNormalTexture.Sample(PointSampler, ScreenUV);
    float4 DecalMasks = DecalMasksTexture.Sample(PointSampler, ScreenUV);
    
    float3 BaseColor = BaseColorTexture.Sample(BaseColorSampler, IN.UV1).xyz;
    
    float2 GrungeUV = IN.UV1 + IN.Rand.xx;
    float Grunge = GrungeTexture.Sample(GrungeSampler, GrungeUV).r;
    //BaseColor = lerp(BaseColor, Parameters.ColorTint.xyz, Parameters.ColorTint.a);
    BaseColor += (Grunge - 0.5) * Parameters.GrungeStrength;
    
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