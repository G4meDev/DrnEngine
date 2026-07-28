#include "Common.hlsl"

// DOMAIN_SURFACE
// BLEND_ADDITIVE
// SHADING_LIT

// SUPPORT_PARTICLE_SPRITE
// SUPPORT_MAIN_PASS
// SUPPORT_DISTORTION

ConstantBuffer<TranslucentResources> BindlessResources : register(b0);

struct ParametersBuffers
{
    TEX2D(Burst, BurstTexture)
};

struct VertexShaderOutput
{
    float4 Color : COLOR;
    float2 UV : UV;
    float SubImage : SUBUV;
    float3x3 TangentToWorld : TNB;
    float3 WorldPosition : WORLDPOS;
    float4 Position : SV_Position;
};

struct PixelShaderOutput
{
#if TRANSLUCENCY_PASS
    float4 TranslucentColor;
#elif DISTORTION_PASS
    float4 Distortion;
#endif
};

VertexShaderOutput Main_VS(VertexInputParticleSprite IN)
{
    VertexShaderOutput OUT;
    
    ConstantBuffer<ViewBuffer> View = ResourceDescriptorHeap[BindlessResources.ViewIndex];
    ConstantBuffer<ParticleSpriteBuffer> ParticleBuffer = ResourceDescriptorHeap[BindlessResources.PrimitiveIndex];

    float4 WorldPosition = mul(ParticleBuffer.SimulationToWorld, float4(IN.ParticlePosition_RelativeTime.xyz, 1.0f));
    float4 OldWorldPosition = mul(ParticleBuffer.SimulationToWorld, float4(IN.ParticleOldPosition_Id.xyz, 1.0f));
    
    float3 Right, Up;
    ParticleSpriteTangents(View, IN.Size_Rotation_Subindex.z, WorldPosition.xyz, OldWorldPosition.xyz, Right, Up, false);

    
    float2 Size = abs(IN.Size_Rotation_Subindex.xy);
    Size = Size.xx;
    WorldPosition.xyz += Size.x * (IN.Position.x - 0.5f) * Right;
    WorldPosition.xyz += Size.y * (IN.Position.y - 0.5f) * Up;

    float3x3 TangentToWorld = ParticleSpriteCalcTangentBasis(Right, Up);
    
    OUT.WorldPosition = WorldPosition.xyz;
    OUT.Position = mul(View.WorldToProjection, WorldPosition);
    OUT.TangentToWorld = TangentToWorld;
    OUT.Color = IN.ParticleColor;
    OUT.UV = IN.Position;
    OUT.SubImage = IN.Size_Rotation_Subindex.w;
    
    return OUT;
}

// -------------------------------------------------------------------------------------

struct PixelShaderInput
{
    float4 Color : COLOR;
    float2 UV : UV;
    float SubImage : SUBUV;
    float3x3 TangentToWorld : TNB;
    float3 WorldPosition : WORLDPOS;
    float4 Position : SV_Position;
};

PixelShaderOutput Main_PS(PixelShaderInput IN) : SV_Target
{
    ConstantBuffer<ParametersBuffers> Parameters = ResourceDescriptorHeap[BindlessResources.ParametersBufferIndex];
    ConstantBuffer<ViewBuffer> View = ResourceDescriptorHeap[BindlessResources.ViewIndex];
    ConstantBuffer<StaticSamplers> StaticSamplers = ResourceDescriptorHeap[BindlessResources.StaticSamplerBufferIndex];
    ConstantBuffer<GBufferTextures> GbufferTextures = ResourceDescriptorHeap[BindlessResources.GbufferTextureIndex];
    
    SamplerState LinearSampler = ResourceDescriptorHeap[StaticSamplers.LinearSamplerIndex];
    SamplerState PointSampler = ResourceDescriptorHeap[StaticSamplers.PointClampIndex];
    
    float2 ScreenUV = SvPositionToViewportUV(IN.Position.xy, View.InvSize);
    Texture2D DepthTexture = ResourceDescriptorHeap[GbufferTextures.DepthIndex];
    float SceneDepth = ConvertFromDeviceZ(DepthTexture.Sample(PointSampler, ScreenUV).r, View.InvDeviceZToWorldZTransform);
    float PixelDepth = ConvertFromDeviceZ(IN.Position.z, View.InvDeviceZToWorldZTransform);
    
    Texture2D BurstTexture = ResourceDescriptorHeap[Parameters.Burst_Texture];
    SamplerState BurstSampler = ResourceDescriptorHeap[Parameters.Burst_Sampler];
    
    float4 Burst = BurstTexture.Sample(BurstSampler, IN.UV);

    float Opacity = lerp(12, 1, IN.Color.a);
    Opacity = pow(Burst.g + Burst.b, Opacity);
    Opacity *= lerp(8, 1, IN.Color.a);
    
    float Refraction = saturate(Opacity * 2);
    Refraction = lerp(1, 1.2, Refraction);
    
    Opacity = saturate(Opacity);
    float3 Color = IN.Color.rgb;
    float4 OutColor = float4(Color, Opacity);
    
// -------------------------------------------------------------------------------------------------------------

    float3 Normal = normalize(mul(float3(0.8, 1.0, 0.8f), IN.TangentToWorld));
    
    float2 BufferUVDistortion = ComputeBufferUVDistortion(View, Normal, Refraction);
    float2 DistortBufferUV = ScreenUV + BufferUVDistortion;

    float DistortSceneDepth = ConvertFromDeviceZ(DepthTexture.Sample(PointSampler, DistortBufferUV).r, View.InvDeviceZToWorldZTransform);
    PostProcessUVDistortion(IN.Position, DistortSceneDepth, 0, BufferUVDistortion);

    float2 PosOffset = max(BufferUVDistortion, 0);
    float2 NegOffset = abs(min(BufferUVDistortion, 0));

    float4 Distortion = float4(PosOffset.x, PosOffset.y, NegOffset.x, NegOffset.y);
    
// -------------------------------------------------------------------------------------------------------------
    
    PixelShaderOutput OUT;
    
#if TRANSLUCENCY_PASS
    OUT.TranslucentColor = OutColor;
#elif DISTORTION_PASS
    OUT.Distortion = Distortion;
#endif
    
    return OUT;
}