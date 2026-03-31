//#define INSTANCED 1
//#define STATICMESH 1
#include "Common.hlsl"

// DOMAIN_SURFACE
// BLEND_TRANSLUCENT
// SHADING_UNLIT

// SUPPORT_STATICMESH
// SUPPORT_INSTANCED

// S-----UPPORT_HIT_PROXY_PASS
// S-----UPPORT_EDITOR_SELECTION_PASS

// TWO_SIDED

ConstantBuffer<StandardResources> BindlessResources : register(b0);

struct ParametersBuffers
{
    VECTOR(ColorTint, ColorTint)
    SCALAR(RoughnessMultiplier, RoughnessMultiplier)
    SCALAR(NormalStrength, NormalStrength)
    SCALAR(OpacityA, OpacityA)
    SCALAR(OpacityB, OpacityB)
    SCALAR(Opacity, Opacity)
    SCALAR(OpacityFresnelPower, OpacityFresnelPower)
    
    TEX2D(BaseColor, BaseColorTexture)
    TEX2D(Normal, NormalTexture)
    TEX2D(Masks, MasksTexture)
};

struct VertexShaderOutput
{
    float3x3 TBN : TBN;
    float2 UV1 : TEXCOORD1;
    float3 WorldPosition : POS;
    float4 Position : SV_Position;
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
    ConstantBuffer<PrimitiveBuffer> P = ResourceDescriptorHeap[BindlessResources.PrimitiveIndex];

    matrix LocalToWorld;
    
#if STATICMESH
    LocalToWorld = P.LocalToWorld;
   
#elif INSTANCED
    LocalToWorld = GetLocalToWorld(IN);
#endif
    
    float4 WorldPosition = mul(LocalToWorld, float4(IN.Position, 1));
    
    float3 WorldNormal = normalize(mul((float3x3)LocalToWorld, IN.Normal));
    float3 WorldTangent = normalize(mul((float3x3)LocalToWorld, IN.Tangent));
    OUT.TBN = GetTBN(WorldNormal, WorldTangent);
    
    OUT.WorldPosition = WorldPosition.xyz;
    OUT.Position = mul(View.WorldToProjection, WorldPosition);
    OUT.UV1 = IN.UV1;
    
    return OUT;
}

//// -------------------------------------------------------------------------------------

struct PixelShaderInput
{
    float3x3 TBN : TBN;
    float2 UV1 : TEXCOORD1;
    float3 WorldPosition : POS;
    float4 Position : SV_Position;
};

struct PixelShaderOutput
{
#if TRANSLUCENCY_PASS
    float4 TranslucentColor;
#elif HITPROXY_PASS
    uint4 Guid;
#elif EDITOR_PRIMITIVE_PASS
    float4 Color;
#elif SHADOW_PASS
#endif
};

//#define TRANSLUCENCY_PASS 1

PixelShaderOutput Main_PS(PixelShaderInput IN, bool FrontFace : SV_IsFrontFace) : SV_Target
{
    PixelShaderOutput OUT;
 
#if TRANSLUCENCY_PASS

    ConstantBuffer<ViewBuffer> View = ResourceDescriptorHeap[BindlessResources.ViewIndex];
    
    ConstantBuffer<StaticSamplers> StaticSamplers = ResourceDescriptorHeap[BindlessResources.StaticSamplerBufferIndex];
    SamplerState LinearSampler = ResourceDescriptorHeap[StaticSamplers.LinearSamplerIndex];
    SamplerState PointSampler = ResourceDescriptorHeap[StaticSamplers.PointSamplerIndex];
    
    ConstantBuffer<ParametersBuffers> Parameters = ResourceDescriptorHeap[BindlessResources.ParametersBufferIndex];

    Texture2D BaseColorTexture = ResourceDescriptorHeap[Parameters.BaseColor_Texture];
    SamplerState BaseColorSampler = ResourceDescriptorHeap[Parameters.BaseColor_Sampler];
    
    Texture2D NormalTexture = ResourceDescriptorHeap[Parameters.Normal_Texture];
    SamplerState NormalSampler = ResourceDescriptorHeap[Parameters.Normal_Sampler];
    
    Texture2D MasksTexture = ResourceDescriptorHeap[Parameters.Masks_Texture];
    SamplerState MasksSampler = ResourceDescriptorHeap[Parameters.Masks_Sampler];
        
    float3 BaseColor = BaseColorTexture.Sample(BaseColorSampler, IN.UV1).xyz;
    BaseColor = lerp(BaseColor, Parameters.ColorTint.rgb, Parameters.ColorTint.a);

    float3 Masks = MasksTexture.Sample(MasksSampler, IN.UV1).xyz;
    Masks.g *= Parameters.RoughnessMultiplier;
    
    float3 Normal = NormalTexture.Sample(NormalSampler, IN.UV1).rgb;
    Normal = ReconstructTextureNormal(Normal.xy, false);
    Normal = lerp(float3(0.0f, 1.0f, 0.0f), Normal, Parameters.NormalStrength);
    Normal = normalize(mul(Normal, IN.TBN));

    if(!FrontFace)
    {
        Normal = -Normal;
    }
    float2 N = EncodeNormal(Normal);
    
    float3 CameraVector = normalize(View.CameraPos - IN.WorldPosition);
    float OpacityFresnel = Fresnel_Function(Normal, CameraVector, Parameters.OpacityFresnelPower);
    float Opacity = lerp(Parameters.OpacityA, Parameters.OpacityB, OpacityFresnel) * Parameters.Opacity;
    Opacity = saturate(Opacity);
    
    OUT.TranslucentColor = float4(BaseColor, Opacity);
    
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