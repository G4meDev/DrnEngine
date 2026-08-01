#include "Common.hlsl"

// DOMAIN_SURFACE
// BLEND_TRANSLUCENT
// SHADING_LIT

// SUPPORT_PARTICLE_SPRITE
// SUPPORT_MAIN_PASS

// TWO_SIDED

ConstantBuffer<TranslucentResources> BindlessResources : register(b0);

struct ParametersBuffers
{
    TEX2D(Color, ColorTexture)
    TEX2D(Normal, NormalTexture)
};

struct VertexShaderOutput
{
    float4 Color : COLOR;
    float2 UV : UV;
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
    Right = ParticleBuffer.EmitterRight;
    Up = ParticleBuffer.EmitterForward;
    //ParticleSpriteTangents(View, IN.Size_Rotation_Subindex.z, WorldPosition.xyz, OldWorldPosition.xyz, Right, Up, false);
    
    float2 Size = abs(IN.Size_Rotation_Subindex.xy);
    WorldPosition.xyz += Size.x * (IN.Position.x - 0.5f) * Right;
    WorldPosition.xyz += Size.y * (IN.Position.y - 0.5f) * Up;
    
    float3x3 TangentToWorld = ParticleSpriteCalcTangentBasis(Right, Up);
    
    OUT.TangentToWorld = TangentToWorld;
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
    float3x3 TangentToWorld : TNB;
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
    
    Texture2D ColorTexture = ResourceDescriptorHeap[Parameters.Color_Texture];
    SamplerState ColorSampler = ResourceDescriptorHeap[Parameters.Color_Sampler];
    
    Texture2D NormalTexture = ResourceDescriptorHeap[Parameters.Normal_Texture];
    SamplerState NormalSampler = ResourceDescriptorHeap[Parameters.Normal_Sampler];
    
    float2 Uv = IN.UV.yx * float2(1, -1);
    
    float4 ColorSample = ColorTexture.Sample(ColorSampler, Uv);
    float4 NormalSample = NormalTexture.Sample(NormalSampler, Uv);
    
    float Opacity = saturate(IN.Color.a * ColorSample.r);
    float3 Color = IN.Color.rgb;
    
    float2 Normal = NormalSample.xy;
    Normal.x = 1 - NormalSample.x;
    float3 TangentNormal = ReconstructTextureNormal(Normal * 1, true);
    float3 WorldNormal = normalize(mul(TangentNormal, IN.TangentToWorld));

// -------------------------------------------------------------------------------------------------------------
    
    GBufferData GBuffer;
    GBuffer.BaseColor = Color;
    GBuffer.WorldNormal = WorldNormal;
    GBuffer.Matallic = 0;
    GBuffer.Roughness = 0.5f;
    GBuffer.AmbientOcclusion = 1.0f;
    GBuffer.TransmittanceColor = Color;
    GBuffer.ShadingModel = SHADING_MODEL_LIT;
    
    float2 PixelPosition = IN.Position.xy;
    float PixelDepth = ConvertFromDeviceZ(IN.Position.z, View.InvDeviceZToWorldZTransform);
    
    ConstantBuffer<LightGridData> LightGrid = ResourceDescriptorHeap[BindlessResources.LightGridIndex];
    float4 OutColor = float4(CalculateLightingForTranslucency(View, LightGrid, GBuffer, IN.WorldPosition, PixelPosition, PixelDepth), Opacity);
    
// -------------------------------------------------------------------------------------------------------------
    
#if TRANSLUCENCY_PASS
    OUT.TranslucentColor = OutColor;
#endif
    
    return OUT;
}