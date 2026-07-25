#include "Common.hlsl"

// DOMAIN_SURFACE
// BLEND_TRANSLUCENT
// SHADING_UNLIT

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
    WorldPosition.xyz += Size.x * (IN.Position.x - 0.5f) * Right;
    WorldPosition.xyz += Size.y * (IN.Position.y - 0.5f) * Up;
    
    float3x3 TangentToWorld = ParticleSpriteCalcTangentBasis(Right, Up);
    
    OUT.Position = mul(View.WorldToProjection, WorldPosition);
    OUT.Color = IN.ParticleColor;
    OUT.UV = IN.Position;
    OUT.SubImage = IN.ParticlePosition_RelativeTime.w * 63;
    
    return OUT;
}

// -------------------------------------------------------------------------------------

struct PixelShaderInput
{
    float4 Color : COLOR;
    float2 UV : UV;
    float SubImage : SUBUV;
    float4 Position : SV_Position;
};

float4 TextureSubuv(Texture2D Texture, SamplerState State, float2 Uv, float2 SubImages, float Frame, bool LinearBlend = false)
{
    [flatten]
    if(LinearBlend)
    {
        float2 InvSubImages = 1.0f / SubImages;
        float Frac = frac(Frame);
        float Integral = Frame - Frac;
        
        float2 Offset1 = float2(fmod(Integral, SubImages.x), floor(Integral * InvSubImages.x));
        float2 Uv1 = (Uv + Offset1) * InvSubImages;
        float4 Sample1 = Texture.Sample(State, Uv1);
        
        float2 Offset2 = float2(fmod(Integral + 1, SubImages.x), floor((Integral + 1) * InvSubImages.x));
        float2 Uv2 = (Uv + Offset2) * InvSubImages;
        float4 Sample2 = Texture.Sample(State, Uv2);
        
        return lerp(Sample1, Sample2, Frac);
    }
    else
    {
        float2 InvSubImages = 1.0f / SubImages;
        float Frac = frac(Frame);
        float Integral = Frame - Frac;
        
        float2 Offset1 = float2(fmod(Integral, SubImages.x), floor(Integral * InvSubImages.x));
        float2 Uv1 = (Uv + Offset1) * InvSubImages;
        float4 Sample1 = Texture.Sample(State, Uv1);
        
        return Sample1;
    }
}

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
    
    float4 OutColor = float4(Color, Opacity);
    
    PixelShaderOutput OUT;
    
#if TRANSLUCENCY_PASS
    OUT.TranslucentColor = OutColor;
#endif
    
    return OUT;
}