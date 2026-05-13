//#define INSTANCED 1
//#define STATICMESH 1
#include "Common.hlsl"

// DOMAIN_SURFACE
// BLEND_TRANSLUCENT
// SHADING_LIT

// SUPPORT_STATICMESH
// SUPPORT_INSTANCED

// S-----UPPORT_HIT_PROXY_PASS
// S-----UPPORT_EDITOR_SELECTION_PASS
// SUPPORT_DISTORTION

ConstantBuffer<TranslucentResources> BindlessResources : register(b0);

struct ParametersBuffers
{
    VECTOR(ColorTint, ColorTint)
    SCALAR(Roughness, Roughness)
    SCALAR(NormalStrength, NormalStrength)
    SCALAR(OpacityA, OpacityA)
    SCALAR(OpacityB, OpacityB)
    SCALAR(Opacity, Opacity)
    SCALAR(OpacityFresnelPower, OpacityFresnelPower)
    SCALAR(IOR, IOR)
    SCALAR(RefractionBias, RefractionBias)
    SCALAR(PanningSpeed, PanningSpeed)
    SCALAR(WaveScale, WaveScale)
    SCALAR(DepthFade, DepthFade)
    
    TEX2D(Normal, NormalTexture)
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
#elif DISTORTION_PASS
    float4 Distortion;
#elif HITPROXY_PASS
    uint4 Guid;
#elif EDITOR_PRIMITIVE_PASS
    float4 Color;
#elif SHADOW_PASS
#endif
};

//#define TRANSLUCENCY_PASS 1
//#define DISTORTION_PASS 1

PixelShaderOutput Main_PS(PixelShaderInput IN, bool FrontFace : SV_IsFrontFace) : SV_Target
{
    PixelShaderOutput OUT;
 
    ConstantBuffer<ViewBuffer> View = ResourceDescriptorHeap[BindlessResources.ViewIndex];
    ConstantBuffer<PrimitiveBuffer> P = ResourceDescriptorHeap[BindlessResources.PrimitiveIndex];
    ConstantBuffer<GBufferTextures> GbufferTextures = ResourceDescriptorHeap[BindlessResources.GbufferTextureIndex];
    
    Texture2D DepthTexture = ResourceDescriptorHeap[GbufferTextures.DepthIndex];
    float2 ScreenUV = SvPositionToViewportUV(IN.Position.xy, View.InvSize);
    
    ConstantBuffer<StaticSamplers> StaticSamplers = ResourceDescriptorHeap[BindlessResources.StaticSamplerBufferIndex];
    SamplerState LinearSampler = ResourceDescriptorHeap[StaticSamplers.LinearClampIndex];
    SamplerState PointSampler = ResourceDescriptorHeap[StaticSamplers.PointClampIndex];
    
    ConstantBuffer<ParametersBuffers> Parameters = ResourceDescriptorHeap[BindlessResources.ParametersBufferIndex];

    Texture2D NormalTexture = ResourceDescriptorHeap[Parameters.Normal_Texture];
    SamplerState NormalSampler = ResourceDescriptorHeap[Parameters.Normal_Sampler];
    
    float2 PixelPosition = IN.Position.xy;
    float PixelDepth = ConvertFromDeviceZ(IN.Position.z, View.InvDeviceZToWorldZTransform);

    float3 BaseColor = Parameters.ColorTint.xyz;

    float3 Masks = float3(0, Parameters.Roughness, 1);
    
    float TimeOffset = View.GameTime * Parameters.PanningSpeed;
    
    float2 Normal_1 = NormalTexture.Sample(NormalSampler, IN.UV1 * Parameters.WaveScale + TimeOffset * float2(-0.7f, -0.1f)).rg;
    float2 Normal_2 = NormalTexture.Sample(NormalSampler, IN.UV1 * Parameters.WaveScale + TimeOffset * float2(0.6f, 0.1f)).rg;
    
    float3 Normal = ReconstructTextureNormal(Normal_1, false) + ReconstructTextureNormal(Normal_2, false);
    Normal *= float3(Parameters.NormalStrength, 1, Parameters.NormalStrength);
    Normal = normalize(mul(Normal, IN.TBN));

    float2 N = EncodeNormal(Normal);
    
    float3 CameraVector = normalize(View.CameraPos - IN.WorldPosition);
    float OpacityFresnel = Fresnel_Function(Normal, CameraVector, Parameters.OpacityFresnelPower);
    float Opacity = lerp(Parameters.OpacityA, Parameters.OpacityB, OpacityFresnel) * Parameters.Opacity;
    Opacity = saturate(Opacity);

    float SceneDepth = ConvertFromDeviceZ(DepthTexture.Sample(PointSampler, ScreenUV).r, View.InvDeviceZToWorldZTransform);
    float DepthFade = saturate(abs(SceneDepth - PixelDepth) / Parameters.DepthFade);
    Opacity = lerp(0, Opacity, DepthFade);
    
// -------------------------------------------------------------------------------------------------------------
    
    GBufferData GBuffer;
    GBuffer.BaseColor = BaseColor;
    GBuffer.WorldNormal = Normal;
    GBuffer.Matallic = Masks.r;
    GBuffer.Roughness = Masks.g;
    GBuffer.AmbientOcclusion = Masks.b;
    GBuffer.TransmittanceColor = BaseColor;
    //GBuffer.ShadingModel = SHADING_MODEL_LIT;
    GBuffer.ShadingModel = SHADING_MODEL_FOLIAGE;
    
    ConstantBuffer<LightGridData> LightGrid = ResourceDescriptorHeap[BindlessResources.LightGridIndex];
    float4 OutColor = float4(CalculateLightingForTranslucency(View, LightGrid, GBuffer, IN.WorldPosition, PixelPosition, PixelDepth), Opacity);
    OutColor.xyz += GetEnvironemntReflection(View, LightGrid, GBuffer, 0, IN.WorldPosition, PixelPosition, PixelDepth, LinearSampler);
    
// -------------------------------------------------------------------------------------------------------------
    
    float IOR = Parameters.IOR;
    float RefractionBias = Parameters.RefractionBias;
    
    float2 BufferUVDistortion = ComputeBufferUVDistortion(View, Normal, IOR);
    float2 DistortBufferUV = ScreenUV + BufferUVDistortion;

    float DistortSceneDepth = ConvertFromDeviceZ(DepthTexture.Sample(PointSampler, DistortBufferUV).r, View.InvDeviceZToWorldZTransform);
    PostProcessUVDistortion(IN.Position, DistortSceneDepth, RefractionBias, BufferUVDistortion);

    float2 PosOffset = max(BufferUVDistortion, 0);
    float2 NegOffset = abs(min(BufferUVDistortion, 0));

    float4 Distortion = float4(PosOffset.x, PosOffset.y, NegOffset.x, NegOffset.y);
    
#if TRANSLUCENCY_PASS
    OUT.TranslucentColor = OutColor;
#elif DISTORTION_PASS
    OUT.Distortion = Distortion;
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