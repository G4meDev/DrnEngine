//#define STATICMESH 1
#include "Common.hlsl"

// DOMAIN_SURFACE
// BLEND_TRANSLUCENT
// SHADING_UNLIT

// SUPPORT_STATICMESH

ConstantBuffer<TranslucentResources> BindlessResources : register(b0);

struct ParametersBuffers
{
    VECTOR(FogColor, FogColor)

    SCALAR(Opacity, Opacity)
    SCALAR(FadeDistance, FadeDistance)
    
    TEX2D(Falloff, Falloff)
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
 
    ConstantBuffer<ViewBuffer> View = ResourceDescriptorHeap[BindlessResources.ViewIndex];
    ConstantBuffer<PrimitiveBuffer> P = ResourceDescriptorHeap[BindlessResources.PrimitiveIndex];
    ConstantBuffer<GBufferTextures> GbufferTextures = ResourceDescriptorHeap[BindlessResources.GbufferTextureIndex];
    
    ConstantBuffer<StaticSamplers> StaticSamplers = ResourceDescriptorHeap[BindlessResources.StaticSamplerBufferIndex];
    SamplerState LinearSampler = ResourceDescriptorHeap[StaticSamplers.LinearClampIndex];
    SamplerState PointSampler = ResourceDescriptorHeap[StaticSamplers.PointClampIndex];

    float2 ScreenUV = SvPositionToViewportUV(IN.Position.xy, View.InvSize);
    Texture2D DepthTexture = ResourceDescriptorHeap[GbufferTextures.DepthIndex];
    float SceneDepth = ConvertFromDeviceZ(DepthTexture.Sample(PointSampler, ScreenUV).r, View.InvDeviceZToWorldZTransform);
    
    ConstantBuffer<ParametersBuffers> Parameters = ResourceDescriptorHeap[BindlessResources.ParametersBufferIndex];

    Texture2D FalloffTexture = ResourceDescriptorHeap[Parameters.Falloff_Texture];
    SamplerState FalloffSampler = ResourceDescriptorHeap[Parameters.Falloff_Sampler];
    
    float3 VertexNormal = IN.TBN[1];
    float3 CameraVector = normalize(View.CameraPos - IN.WorldPosition);
    float2 PixelPosition = IN.Position.xy;
    float PixelDepth = ConvertFromDeviceZ(IN.Position.z, View.InvDeviceZToWorldZTransform);

    float3 BaseColor = Parameters.FogColor.rgb;
    
    float FacingMask = saturate(pow(dot(VertexNormal, CameraVector), 3));
    float FalloffMask = saturate(IN.UV1.g * FalloffTexture.Sample(FalloffSampler, IN.UV1).r);
    FalloffMask = DepthFade(SceneDepth, PixelDepth, FalloffMask, Parameters.FadeDistance);
    float Opacity = Parameters.Opacity * FalloffMask * FacingMask;
    
// -------------------------------------------------------------------------------------------------------------

    float4 OutColor = float4(BaseColor, Opacity);
    
#if TRANSLUCENCY_PASS
    OUT.TranslucentColor = OutColor;
#elif HITPROXY_PASS
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