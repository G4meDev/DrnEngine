#include "Common.hlsl"

// DOMAIN_SURFACE

// SUPPORT_STATICMESH

// SUPPORT_MAIN_PASS
// SUPPORT_PRE_PASS
// SUPPORT_HIT_PROXY_PASS
// SUPPORT_EDITOR_SELECTION_PASS
// SUPPORT_SHADOW_PASS

// HAS_CUSTOM_PRE_PASS
// HAS_OPACITY

struct Resources
{
    uint ViewIndex;
    uint PrimitiveIndex;
    uint StaticSamplerBufferIndex;
    uint ParametersBufferIndex;
    uint unused_1;
    uint unused_2;
    uint ShadowDepthBuffer;
    uint DecalBaseColor;
    uint DecalNormal;
    uint DecalMasks;
};

ConstantBuffer<Resources> BindlessResources : register(b0);

struct Primitive
{
    matrix LocalToWorld;
    matrix LocalToProjection;
    uint4 Guid;
    matrix PrevLocalToWorld;
    matrix PrevLocalToProjection;
};

struct ParametersBuffers
{
    VECTOR(Color, Color)
    SCALAR(Metallic, Metallic)
    SCALAR(Roughness, Roughness)
    
    TEX2D(Opacity, OpacityTexture)
};

struct VertexShaderOutput
{
    float2 UV1 : TEXCOORD1;
    float4 Position : SV_Position;

#if MAIN_PASS
    float3x3 TBN : TBN;
#endif
};

VertexShaderOutput Main_VS(VertexInputStaticMesh IN)
{
    VertexShaderOutput OUT;
    
#if SHADOW_PASS_POINTLIGHT
    ConstantBuffer<PrimitiveBuffer> P = ResourceDescriptorHeap[BindlessResources.PrimitiveIndex];
    OUT.Position = mul(P.LocalToWorld, float4(IN.Position, 1.0f));
#elif SHADOW_PASS_SPOTLIGHT
    ConstantBuffer<PrimitiveBuffer> P = ResourceDescriptorHeap[BindlessResources.PrimitiveIndex];
    ConstantBuffer<ShadowDepth> ShadowBuffer = ResourceDescriptorHeap[BindlessResources.ShadowDepthBuffer];
    float3 WorldPosition = mul(P.LocalToWorld, float4(IN.Position, 1.0f)).xyz;
    OUT.Position = mul(ShadowBuffer.WorldToProjectionMatrix, float4(WorldPosition, 1));
#elif PRE_PASS
    ConstantBuffer<PrimitiveBuffer> P = ResourceDescriptorHeap[BindlessResources.PrimitiveIndex];
    OUT.Position = mul(P.LocalToProjection, float4(IN.Position, 1.0f));
#elif MAIN_PASS
    ConstantBuffer<PrimitiveBuffer> P = ResourceDescriptorHeap[BindlessResources.PrimitiveIndex];
    ConstantBuffer<ViewBuffer> View = ResourceDescriptorHeap[BindlessResources.ViewIndex];
    ConstantBuffer<ParametersBuffers> Parameters = ResourceDescriptorHeap[BindlessResources.ParametersBufferIndex];
    
    float3 WorldNormal = normalize(mul((float3x3) P.LocalToWorld, IN.Normal));
    float3 WorldTangent = normalize(mul((float3x3) P.LocalToWorld, IN.Tangent));
    OUT.TBN = GetTBN(WorldNormal, WorldTangent);
    
    OUT.Position = mul(P.LocalToProjection, float4(IN.Position, 1.0f));
#else
    ConstantBuffer<PrimitiveBuffer> P = ResourceDescriptorHeap[BindlessResources.PrimitiveIndex];
    OUT.Position = mul(P.LocalToProjection, float4(IN.Position, 1.0f));
#endif
    
    OUT.UV1 = IN.UV1;
    return OUT;
}

//// -------------------------------------------------------------------------------------

struct PixelShaderInput
{
    float2 UV1 : TEXCOORD1;
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
    
    Texture2D OpacityTexture = ResourceDescriptorHeap[Parameters.Opacity_Texture];
    SamplerState OpacitySampler = ResourceDescriptorHeap[Parameters.Opacity_Sampler];
    
    float Opacity = OpacityTexture.Sample(OpacitySampler, IN.UV1).r;
    clip(Opacity - 0.33f);
    
#if MAIN_PASS

    float3 Normal = float3(0, 1, 0);
    Normal = normalize(mul(Normal, IN.TBN));
    if (!FrontFace)
    {
        Normal = -Normal;
    }
    float2 N = EncodeNormal(Normal);
    
    OUT.ColorDeferred = float4(0.0, 0.0, 0.0, 1);
    OUT.BaseColor = float4(Parameters.Color.rgb * Parameters.Color.a, 1);
    OUT.WorldNormal = N;
    OUT.Masks = float4(Parameters.Metallic, Parameters.Roughness, 1, Uint8ToFloat(SHADING_MODEL_FOLIAGE));
    OUT.MasksB = float4(OUT.BaseColor.rgb * 0.7, 1);
    
#elif HITPROXY_PASS
    ConstantBuffer<PrimitiveBuffer> P = ResourceDescriptorHeap[BindlessResources.PrimitiveIndex];
    OUT.Guid = P.Guid;
#elif PRE_PASS || SHADOW_PASS
    
#endif
    
    return OUT;
}

// -------------------------------------------------------------------------------------

struct GeometeryShaderOutput
{
    float2 UV1 : TEXCOORD1;
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
            OUT.UV1 = input[VertexIndex].UV1;
            OutputStream.Append(OUT);
        }
		OutputStream.RestartStrip();
    }
}

#endif