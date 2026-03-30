//#define INSTANCED 1
//#define STATICMESH 1
#include "Common.hlsl"

// DOMAIN_SURFACE
// BLEND_MASKED
// SHADING_LIT

// SUPPORT_STATICMESH
// SUPPORT_INSTANCED

// SUPPORT_MAIN_PASS
// SUPPORT_PRE_PASS
// SUPPORT_HIT_PROXY_PASS
// SUPPORT_EDITOR_SELECTION_PASS
// SUPPORT_SHADOW_PASS

// HAS_CUSTOM_PRE_PASS
// TWO_SIDED

ConstantBuffer<StandardResources> BindlessResources : register(b0);

struct ParametersBuffers
{
    SCALAR(RoughnessMultiplier, RoughnessMultiplier)
    SCALAR(NormalStrength, NormalStrength)
    SCALAR(FlatNormalStrength, FlatNormalStrength)
    SCALAR(SubsurfaceColorIntensity, SubsurfaceColorIntensity)
    SCALAR(CullDistance, CullDistance)
    
    TEX2D(BaseColorOpacity, BaseColorOpacityTexture)
    TEX2D(Normal, NormalTexture)
    TEX2D(Masks, MasksTexture)
};

struct VertexShaderOutput
{
    float2 UV1 : TEXCOORD1;
    float ClipValue : DATA1;
    float4 Position : SV_Position;
    
#if MAIN_PASS
    float3x3 TBN : TBN;
#endif
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
    ConstantBuffer<ParametersBuffers> Parameters = ResourceDescriptorHeap[BindlessResources.ParametersBufferIndex];
    
    matrix LocalToWorld;
    
#if STATICMESH
    LocalToWorld = P.LocalToWorld;
#elif INSTANCED
    LocalToWorld = GetLocalToWorld(IN);
#endif
    
    float4 WorldPosition = mul(LocalToWorld, float4(IN.Position, 1));
    OUT.ClipValue = Parameters.CullDistance - distance(WorldPosition.xyz, View.CameraPos);
    OUT.UV1 = IN.UV1;
    
#if SHADOW_PASS_POINTLIGHT
    OUT.Position = WorldPosition;
#elif SHADOW_PASS_SPOTLIGHT
    ConstantBuffer<ShadowDepth> ShadowBuffer = ResourceDescriptorHeap[BindlessResources.ShadowDepthBuffer];
    OUT.Position = mul(ShadowBuffer.WorldToProjectionMatrix, WorldPosition);
    
#elif PRE_PASS | HITPROXY_PASS
    OUT.Position = mul(View.WorldToProjection, WorldPosition);
    
#elif MAIN_PASS
    
    float3 WorldNormal = normalize(mul((float3x3)LocalToWorld, IN.Normal));
    float3 WorldTangent = normalize(mul((float3x3)LocalToWorld, IN.Tangent));
    OUT.TBN = GetTBN(WorldNormal, WorldTangent);
    
    OUT.Position = mul(View.WorldToProjection, WorldPosition);

#else
    #error invalid pass
#endif
    
    return OUT;
}

//// -------------------------------------------------------------------------------------

struct PixelShaderInput
{
    float2 UV1 : TEXCOORD1;
    float ClipValue : DATA1;
    float4 Position : SV_Position;
    
#if MAIN_PASS
    float3x3 TBN : TBN;
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

PixelShaderOutput Main_PS(PixelShaderInput IN, bool FrontFace : SV_IsFrontFace) : SV_Target
{
    PixelShaderOutput OUT;
 
    ConstantBuffer<ParametersBuffers> Parameters = ResourceDescriptorHeap[BindlessResources.ParametersBufferIndex];

    Texture2D BaseColorOpacityTexture = ResourceDescriptorHeap[Parameters.BaseColorOpacity_Texture];
    SamplerState BaseColorOpacitySampler = ResourceDescriptorHeap[Parameters.BaseColorOpacity_Sampler];
    
    float4 BaseColorOpacity = BaseColorOpacityTexture.Sample(BaseColorOpacitySampler, IN.UV1);
    clip(IN.ClipValue);
    clip(BaseColorOpacity.a - 0.33f);
    
#if MAIN_PASS

    ConstantBuffer<StaticSamplers> StaticSamplers = ResourceDescriptorHeap[BindlessResources.StaticSamplerBufferIndex];
    SamplerState LinearSampler = ResourceDescriptorHeap[StaticSamplers.LinearSamplerIndex];
    SamplerState PointSampler = ResourceDescriptorHeap[StaticSamplers.PointSamplerIndex];
    
    ConstantBuffer<ViewBuffer> View = ResourceDescriptorHeap[BindlessResources.ViewIndex];
    Texture2D DecalBaseColorTexture = ResourceDescriptorHeap[BindlessResources.DecalBaseColor];
    Texture2D DecalNormalTexture = ResourceDescriptorHeap[BindlessResources.DecalNormal];
    Texture2D DecalMasksTexture = ResourceDescriptorHeap[BindlessResources.DecalMasks];
    
    Texture2D NormalTexture = ResourceDescriptorHeap[Parameters.Normal_Texture];
    SamplerState NormalSampler = ResourceDescriptorHeap[Parameters.Normal_Sampler];
    
    Texture2D MasksTexture = ResourceDescriptorHeap[Parameters.Masks_Texture];
    SamplerState MasksSampler = ResourceDescriptorHeap[Parameters.Masks_Sampler];
    
    float2 ScreenUV = SvPositionToViewportUV(IN.Position.xy, View.InvSize);
    float4 DecalBaseColor = DecalBaseColorTexture.Sample(PointSampler, ScreenUV);
    float4 DecalNormal = DecalNormalTexture.Sample(PointSampler, ScreenUV);
    float4 DecalMasks = DecalMasksTexture.Sample(PointSampler, ScreenUV);
    
    //float4 VarColor = IN.InstanceVarIndex > 0.5f ? float4(1, 1, 1, 0.2) : float4(0, 0, 1, 0);
    float3 BaseColor = BaseColorOpacity.xyz;
    //BaseColor = lerp(BaseColor, VarColor.rgb, VarColor.a);
    
    float3 Masks = MasksTexture.Sample(MasksSampler, IN.UV1).xyz;
    Masks.g *= Parameters.RoughnessMultiplier;
    //Masks.g += IN.InstanceRandom * Parameters.RandomRoughnessStrength;
    
    float3 Normal = NormalTexture.Sample(NormalSampler, IN.UV1).rgb;
    Normal = ReconstructTextureNormal(Normal.xy, false);
    Normal = lerp(float3(0.0f, 1.0f, 0.0f), Normal, Parameters.NormalStrength);
    if(!FrontFace)
    {
        Normal = -Normal;
    }
    //Normal = normalize(mul(Normal, IN.TBN));
    Normal = mul(Normal, IN.TBN);
    Normal = lerp(Normal, float3(0, 1, 0), Parameters.FlatNormalStrength);
    Normal = normalize(Normal);
    
    //Normal = lerp(DecalNormal.xyz, Normal, DecalNormal.w);
    float2 N = EncodeNormal(Normal);
    
    
    OUT.ColorDeferred = float4(0.0, 0.0, 0.0, 1);
    OUT.BaseColor = float4(BaseColor, 1);
    OUT.WorldNormal = N;
    OUT.Masks = float4(Masks, Uint8ToFloat(SHADING_MODEL_FOLIAGE));
    
    OUT.MasksB = float4(BaseColor * Parameters.SubsurfaceColorIntensity, 1);
    
    //float2 Velocity = IN.ScreenPos.xy / IN.ScreenPos.w - IN.PrevScreenPos.xy / IN.PrevScreenPos.w;
    //Velocity = Velocity * 0.25f + 0.5f;
    //OUT.Velocity = Velocity;
    //OUT.Velocity = 0;
    
    //OUT.BaseColor.xyz = lerp(DecalBaseColor.xyz, OUT.BaseColor.xyz, DecalBaseColor.w);
    //OUT.Masks.xyz = lerp(DecalMasks.xyz, OUT.Masks.xyz, DecalMasks.w);
    
#elif HITPROXY_PASS
    ConstantBuffer<PrimitiveBuffer> P = ResourceDescriptorHeap[BindlessResources.PrimitiveIndex];
    OUT.Guid = P.Guid;
#endif
    
    return OUT;
}

// -------------------------------------------------------------------------------------

struct GeometeryShaderOutput
{
    float2 UV1 : TEXCOORD1;
    float ClipValue : DATA1;
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
            OUT.ClipValue = input[VertexIndex].ClipValue;
            OUT.UV1 = input[VertexIndex].UV1;
            OutputStream.Append(OUT);
        }
		OutputStream.RestartStrip();
    }
}

#endif