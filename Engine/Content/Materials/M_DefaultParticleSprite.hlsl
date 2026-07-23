#include "Common.hlsl"

// DOMAIN_SURFACE
// BLEND_OPAQUE
// SHADING_LIT

// SUPPORT_PARTICLE_SPRITE

// SUPPORT_MAIN_PASS
// SUPPORT_HIT_PROXY_PASS
// SUPPORT_PRE_PASS
// SUPPORT_EDITOR_SELECTION_PASS

// HAS_CUSTOM_PRE_PASS
// TWO_SIDED

ConstantBuffer<StandardResources> BindlessResources : register(b0);

struct VertexShaderOutput
{
    float4 Color : COLOR;
    float4 Position : SV_Position;
};

struct PixelShaderOutput
{
#if MAIN_PASS
    float4 ColorDeferred : SV_TARGET0;
    float4 BaseColor : SV_TARGET1;
    float4 WorldNormal : SV_TARGET2;
    float4 Masks : SV_TARGET3;
    float4 MasksB : SV_TARGET4;
#elif HITPROXY_PASS
    uint4 Guid;
#elif EDITOR_PRIMITIVE_PASS
    float4 Color;
#endif
};

VertexShaderOutput Main_VS(VertexInputParticleSprite IN)
{
    VertexShaderOutput OUT;
    
    ConstantBuffer<ViewBuffer> View = ResourceDescriptorHeap[BindlessResources.ViewIndex];
    ConstantBuffer<ParticleSpriteBuffer> ParticleBuffer = ResourceDescriptorHeap[BindlessResources.PrimitiveIndex];

    float4 WorldPosition = mul(ParticleBuffer.SimulationToWorld, float4(IN.ParticlePosition_RelativeTime.xyz, 1.0f));
    WorldPosition.xy += IN.Position * IN.Size_Rotation_Subindex.xy;
    OUT.Position = mul(View.WorldToProjection, WorldPosition);
    OUT.Color = float4(IN.ParticleColor.rgb, 1.0f);

    return OUT;
}

// -------------------------------------------------------------------------------------

struct PixelShaderInput
{
    float4 Color : COLOR;
};

PixelShaderOutput Main_PS(PixelShaderInput IN) : SV_Target
{
    ConstantBuffer<ParticleSpriteBuffer> ParticleBuffer = ResourceDescriptorHeap[BindlessResources.PrimitiveIndex];

    PixelShaderOutput OUT;
    
#if MAIN_PASS
    OUT.ColorDeferred = IN.Color;
    OUT.BaseColor = float4(0.7, 0.5, 1, 1);
    OUT.WorldNormal = float4(0, 1, 0, 1);
    OUT.Masks = float4(0.2, 1, 0.4, 1);
#elif HITPROXY_PASS
    OUT.Guid = ParticleBuffer.Guid;
#endif
    
    return OUT;
}