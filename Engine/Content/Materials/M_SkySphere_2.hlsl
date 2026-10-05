#include "Common.hlsl"

// DOMAIN_SURFACE
// BLEND_OPAQUE
// SHADING_UNLIT

// SUPPORT_STATICMESH

// SUPPORT_MAIN_PASS
// SUPPORT_PRE_PASS
// SUPPORT_HIT_PROXY_PASS
// SUPPORT_EDITOR_SELECTION_PASS
// SUPPORT_-_SHADOW_PASS

ConstantBuffer<StandardResources> BindlessResources : register(b0);

struct ParametersBuffers
{
    VECTOR(TintColor, TintColor)
    
    SCALAR(Exposure, Exposure)
    SCALAR(Intensity, Intensity)

    TEXCUBE(BaseColor, BaseColorTexture)
};

struct VertexShaderOutput
{
    float4 Color : COLOR;
    float3 Normal : NORMAL;
    float3x3 TBN : TBN;
    float2 UV : TEXCOORD;
    float4 Position : SV_Position;
};

VertexShaderOutput Main_VS(VertexInputStaticMesh IN)
{
    VertexShaderOutput OUT;

    ConstantBuffer<ViewBuffer> View = ResourceDescriptorHeap[BindlessResources.ViewIndex];
    ConstantBuffer<PrimitiveBuffer> Primitive = ResourceDescriptorHeap[BindlessResources.PrimitiveIndex];
    
    float4 WorldPosition = mul(Primitive.LocalToWorld, float4(IN.Position, 1.0f));
    
    float3 WorldNormal = normalize(mul((float3x3) Primitive.LocalToWorld, IN.Normal));
    float3 WorldTangent = normalize(mul((float3x3) Primitive.LocalToWorld, IN.Tangent));
    OUT.TBN = GetTBN(WorldNormal, WorldTangent);
    
    OUT.Position = mul(View.WorldToProjection, WorldPosition);
    OUT.Color = float4(IN.Color.rgb, 1.0f);
    OUT.Normal = WorldNormal;
    OUT.UV = IN.UV1;
    
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
    float2 UV : TEXCOORD;
#endif
};

struct PixelShaderOutput
{
#if MAIN_PASS
    float4 ColorDeferred : SV_TARGET0;
    float4 Masks : SV_TARGET3;
#elif HITPROXY_PASS
    uint4 Guid : SV_TARGET0;
#elif EDITOR_PRIMITIVE_PASS
    float4 Color : SV_TARGET0;
#elif SHADOW_PASS
#endif
};

//#define MAIN_PASS 1

PixelShaderOutput Main_PS(PixelShaderInput IN)
{
    PixelShaderOutput OUT;
 
#if MAIN_PASS

    ConstantBuffer<StaticSamplers> StaticSamplers = ResourceDescriptorHeap[BindlessResources.StaticSamplerBufferIndex];
    SamplerState LinearSampler = ResourceDescriptorHeap[StaticSamplers.LinearSamplerIndex];
    
    ConstantBuffer<ParametersBuffers> Parameters = ResourceDescriptorHeap[BindlessResources.ParametersBufferIndex];

    TextureCube BaseColorTexture = ResourceDescriptorHeap[Parameters.BaseColor_Texture];
    SamplerState BaseColorSampler = ResourceDescriptorHeap[Parameters.BaseColor_Sampler];

    float3 BaseColor = BaseColorTexture.Sample(BaseColorSampler, -IN.Normal).xyz;
    BaseColor = pow(BaseColor, Parameters.Exposure) * Parameters.Intensity;
    
    OUT.ColorDeferred = float4(BaseColor, 1);
    //OUT.ColorDeferred = pow(OUT.ColorDeferred, 1.0f / 2.2f);
    OUT.Masks = float4(0, 0, 1, Uint8ToFloat(SHADING_MODEL_UNLIT));
    
#elif HITPROXY_PASS
    ConstantBuffer<PrimitiveBuffer> P = ResourceDescriptorHeap[BindlessResources.PrimitiveIndex];
    OUT.Guid = P.Guid;
#endif
    
    return OUT;
}