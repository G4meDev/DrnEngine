#include "Common.hlsl"

// DOMAIN_SURFACE
// BLEND_TRANSLUCENT
// SHADING_LIT

// SUPPORT_PARTICLE_SPRITE
// SUPPORT_MAIN_PASS

ConstantBuffer<TranslucentResources> BindlessResources : register(b0);

struct ParametersBuffers
{
    TEX2D(SmokeTile, SmokeTileTexture)
    TEX2D(SmokeSubuv, SmokeSubuvTexture)
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

    //float3x3 TangentToWorld = ParticleSpriteCalcTangentBasis(Right, Up);
    float3x3 TangentToWorld = ParticleSpriteCalcSphereTangentBasis(ParticleBuffer.NormalsSphereCenter, WorldPosition.xyz, Right, Up);
    //float3x3 TangentToWorld = ParticleSpriteCalcCylinderTangentBasis(ParticleBuffer.NormalsSphereCenter, ParticleBuffer.NormalsCylinderDirection, WorldPosition.xyz, Right, Up);
    
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
    
    Texture2D SmokeTileTexture = ResourceDescriptorHeap[Parameters.SmokeTile_Texture];
    SamplerState SmokeTileSampler = ResourceDescriptorHeap[Parameters.SmokeTile_Sampler];
    
    Texture2D SmokeSubuvTexture = ResourceDescriptorHeap[Parameters.SmokeSubuv_Texture];
    SamplerState SmokeSubuvSampler = ResourceDescriptorHeap[Parameters.SmokeSubuv_Sampler];
    
    float4 SmokeTile = SmokeTileTexture.Sample(SmokeTileSampler, IN.UV);
    float4 SmokeSubuv = TextureSubuv(SmokeSubuvTexture, SmokeSubuvSampler, IN.UV, float2(8, 8), IN.SubImage, true);
    
    float Opacity = saturate(IN.Color.a * SmokeSubuv.a);
    Opacity = DepthFade(SceneDepth, PixelDepth, Opacity, 2);
    
    float3 Color = IN.Color.rgb * (SmokeTile.rgb + SmokeSubuv.rgb);
    
    //float4 OutColor = float4(Color, Opacity);
    
    float3 Normal = IN.TangentToWorld[1];
    //float3 Normal = ReconstructTextureNormal(SmokeTile.xy, true);
    //Normal = normalize(mul(Normal, IN.TangentToWorld));
    
// -------------------------------------------------------------------------------------------------------------
    
    GBufferData GBuffer;
    GBuffer.BaseColor = Color;
    GBuffer.WorldNormal = Normal;
    GBuffer.Matallic = 0.0f;
    GBuffer.Roughness = 0.5f;
    GBuffer.AmbientOcclusion = 1.0f;
    GBuffer.TransmittanceColor = 0.0f;
    GBuffer.ShadingModel = SHADING_MODEL_LIT;
    //GBuffer.ShadingModel = SHADING_MODEL_FOLIAGE;
    
    float2 PixelPosition = IN.Position.xy;
    
    ConstantBuffer<LightGridData> LightGrid = ResourceDescriptorHeap[BindlessResources.LightGridIndex];
    float4 OutColor = float4(CalculateLightingForTranslucency(View, LightGrid, GBuffer, IN.WorldPosition, PixelPosition, PixelDepth), Opacity);
    //OutColor.xyz += GetEnvironemntReflection(View, LightGrid, GBuffer, 0, IN.WorldPosition, PixelPosition, PixelDepth, LinearSampler);
    
// -------------------------------------------------------------------------------------------------------------
    
    PixelShaderOutput OUT;
    
#if TRANSLUCENCY_PASS
    OUT.TranslucentColor = OutColor;
#endif
    
    return OUT;
}