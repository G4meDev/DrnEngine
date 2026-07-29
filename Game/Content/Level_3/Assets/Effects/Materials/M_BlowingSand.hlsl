#include "Common.hlsl"

// DOMAIN_SURFACE
// BLEND_TRANSLUCENT
// SHADING_UNLIT

// SUPPORT_PARTICLE_SPRITE
// SUPPORT_MAIN_PASS

// TWO_SIDED

ConstantBuffer<TranslucentResources> BindlessResources : register(b0);

struct ParametersBuffers
{
    TEX2D(Dust, DustTexture)
    TEX2D(Mask, MaskTexture)
};

struct VertexShaderOutput
{
    float4 Color : COLOR;
    float2 UV : UV;
    float3 WorldPosition : WORLDPOS;
    float4 Position : SV_Position;
};

struct PixelShaderOutput
{
#if TRANSLUCENCY_PASS
    float4 TranslucentColor;
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
    Right = ParticleBuffer.EmitterForward;
    Up = ParticleBuffer.EmitterUp;
    //ParticleSpriteTangents(View, IN.Size_Rotation_Subindex.z, WorldPosition.xyz, OldWorldPosition.xyz, Right, Up, false);
    
    float2 Size = abs(IN.Size_Rotation_Subindex.xy);
    WorldPosition.xyz += Size.x * (IN.Position.x - 0.5f) * Right;
    WorldPosition.xyz += Size.y * (IN.Position.y - 0.5f) * Up;
    
    float3x3 TangentToWorld = ParticleSpriteCalcTangentBasis(Right, Up);
    
    OUT.WorldPosition = WorldPosition.xyz;
    OUT.Position = mul(View.WorldToProjection, WorldPosition);
    OUT.Color = IN.ParticleColor;
    OUT.UV = IN.Position;
    
    return OUT;
}

// -------------------------------------------------------------------------------------

struct PixelShaderInput
{
    float4 Color : COLOR;
    float2 UV : UV;
    float3 WorldPosition : WORLDPOS;
    float4 Position : SV_Position;
};

PixelShaderOutput Main_PS(PixelShaderInput IN) : SV_Target
{
    PixelShaderOutput OUT;
    
    ConstantBuffer<ViewBuffer> View = ResourceDescriptorHeap[BindlessResources.ViewIndex];
    ConstantBuffer<PrimitiveBuffer> P = ResourceDescriptorHeap[BindlessResources.PrimitiveIndex];
    ConstantBuffer<GBufferTextures> GbufferTextures = ResourceDescriptorHeap[BindlessResources.GbufferTextureIndex];
    
    ConstantBuffer<StaticSamplers> StaticSamplers = ResourceDescriptorHeap[BindlessResources.StaticSamplerBufferIndex];
    SamplerState LinearSampler = ResourceDescriptorHeap[StaticSamplers.LinearClampIndex];
    SamplerState PointSampler = ResourceDescriptorHeap[StaticSamplers.PointClampIndex];
    
    ConstantBuffer<ParametersBuffers> Parameters = ResourceDescriptorHeap[BindlessResources.ParametersBufferIndex];
    
    float2 ScreenUV = SvPositionToViewportUV(IN.Position.xy, View.InvSize);
    Texture2D DepthTexture = ResourceDescriptorHeap[GbufferTextures.DepthIndex];
    float SceneDepth = ConvertFromDeviceZ(DepthTexture.Sample(PointSampler, ScreenUV).r, View.InvDeviceZToWorldZTransform);
    float PixelDepth = ConvertFromDeviceZ(IN.Position.z, View.InvDeviceZToWorldZTransform);
    
    Texture2D DustTexture = ResourceDescriptorHeap[Parameters.Dust_Texture];
    SamplerState DustSampler = ResourceDescriptorHeap[Parameters.Dust_Sampler];
    
    Texture2D MaskTexture = ResourceDescriptorHeap[Parameters.Mask_Texture];
    SamplerState MaskSampler = ResourceDescriptorHeap[Parameters.Mask_Sampler];
    
    float2 DustUv1 = Panner(IN.UV.yx, float2(0, -0.5f), View.GameTime);
    float2 DustUv2 = Panner(IN.UV.yx, float2(0, -0.75f), View.GameTime);
    
    float Dust1 = DustTexture.Sample(DustSampler, DustUv1).r;
    float Dust2 = DustTexture.Sample(DustSampler, DustUv2).r;
    float Mask = MaskTexture.Sample(MaskSampler, IN.UV.yx).r;
    
    float CamDist = distance(View.CameraPos, IN.WorldPosition);
    float Opacity = saturate((CamDist - 1) / 30);
    Opacity *= Dust2;
    Opacity = DepthFade(SceneDepth, PixelDepth, Opacity, 4);
    Opacity *= Mask;
    Opacity = saturate(Opacity * IN.Color.a);
    
    float3 Color = IN.Color.rgb * Dust1 * Dust2;
    
    //float4 OutColor = float4(IN.Color.rgb, IN.Color.a * RadialGradientExponential(IN.UV));
    float4 OutColor = float4(Color, Opacity);
    
#if TRANSLUCENCY_PASS
    OUT.TranslucentColor = OutColor;
#endif
    
    return OUT;
}