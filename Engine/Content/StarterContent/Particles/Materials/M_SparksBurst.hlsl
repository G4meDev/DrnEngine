#include "Common.hlsl"

// DOMAIN_SURFACE
// BLEND_ADDITIVE
// SHADING_UNLIT

// SUPPORT_PARTICLE_SPRITE
// SUPPORT_MAIN_PASS

ConstantBuffer<TranslucentResources> BindlessResources : register(b0);

struct ParametersBuffers
{
    TEX2D(SparkBurst, SparkBurstTexture)
};

struct VertexShaderOutput
{
    float4 Color : COLOR;
    float2 UV : UV;
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
    
    float3x3 TangentToWorld = ParticleSpriteCalcTangentBasis(Right, Up);
    
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
};

PixelShaderOutput Main_PS(PixelShaderInput IN) : SV_Target
{
    ConstantBuffer<ParametersBuffers> Parameters = ResourceDescriptorHeap[BindlessResources.ParametersBufferIndex];
    
    Texture2D SparkBurstTexture = ResourceDescriptorHeap[Parameters.SparkBurst_Texture];
    SamplerState SparkBurstSampler = ResourceDescriptorHeap[Parameters.SparkBurst_Sampler];
    
    float4 SparkBurst = SparkBurstTexture.Sample(SparkBurstSampler, IN.UV);
    
    float Opacity = IN.Color.a * SparkBurst.r;
    Opacity = saturate(Opacity);
    float3 Color = IN.Color.rgb * SparkBurst.rgb;
    
    float4 OutColor = float4(Color, Opacity);
    
    PixelShaderOutput OUT;
    
#if TRANSLUCENCY_PASS
    OUT.TranslucentColor = OutColor;
#endif
    
    return OUT;
}