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

ConstantBuffer<StandardResources> BindlessResources : register(b0);

struct ParametersBuffers
{
    TEX2D(BaseColor, BaseColorTexture)
    TEX2D(Normal, NormalTexture)
    TEX2D(Masks, MasksTexture)
};

struct VertexShaderOutput
{
    float4 Color : COLOR;
    float2 UV : UV;
    float3x3 TangentToWorld : TBN;
    float4 Position : SV_Position;
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
#endif
};

void ParticleSpriteTangents(ViewBuffer View, float Rotation, float3 WorldPosition, float3 OldWorldPosition, out float3 OutRight, out float3 OutUp, bool VelocityAlign)
{
    float3 RightVector = View.CameraRightVector;
    float3 UpVector = View.CameraUpVector;
    float3 CameraDirection = SafeNormalize(View.CameraPos - WorldPosition.xyz);
    
    [flatten]
    if (VelocityAlign)
    {
        float3 ParticleDirection = SafeNormalize(WorldPosition.xyz - OldWorldPosition.xyz);
        RightVector = SafeNormalize(cross(CameraDirection, ParticleDirection));
        UpVector = -ParticleDirection;
    }
    
    float SinRotation;
	float CosRotation;
	const float SpriteRotation = Rotation;
	sincos(SpriteRotation, SinRotation, CosRotation);

	OutRight	= SinRotation * UpVector + CosRotation * RightVector;
	OutUp		= CosRotation * UpVector - SinRotation * RightVector;
}

float3x3 ParticleSpriteCalcTangentBasis(float3 Right, float3 Up)
{
    float3 Normal = normalize(cross(Right, -Up));
    return float3x3(Right, Normal, -Up);
}

float ExponentialDenstity(float Depth, float Density, bool UseExp2 = true)
{
    float DC = Depth * Density;
    [flatten]
    if(UseExp2)
    {
        DC *= DC;
    }
    
    if(Depth > 0.0f)
    {
        return 1.0f / pow(2.718, DC);
    }
    
    return 1.0f;
}

float RadialGradientExponential(float2 UV, float2 CenterPosition = float2(0.5, 0.5), float Radius = 0.5, float Density = 2.333, bool bInvertDensity = false)
{
    if(bInvertDensity)
    {
        return ExponentialDenstity(distance(UV, CenterPosition) / Radius, Density);
    }
    else
    {
        return 1 - ExponentialDenstity(1 - (distance(UV, CenterPosition) / Radius), Density);
    }
}

VertexShaderOutput Main_VS(VertexInputParticleSprite IN)
{
    VertexShaderOutput OUT;
    
    ConstantBuffer<ViewBuffer> View = ResourceDescriptorHeap[BindlessResources.ViewIndex];
    ConstantBuffer<ParticleSpriteBuffer> ParticleBuffer = ResourceDescriptorHeap[BindlessResources.PrimitiveIndex];

    float4 WorldPosition = mul(ParticleBuffer.SimulationToWorld, float4(IN.ParticlePosition_RelativeTime.xyz, 1.0f));
    float4 OldWorldPosition = mul(ParticleBuffer.SimulationToWorld, float4(IN.ParticleOldPosition_Id.xyz, 1.0f));
    //float4 OldWorldPosition = WorldPosition - float4(1, 0, 0, 0);
    
    float3 Right, Up;
    ParticleSpriteTangents(View, IN.Size_Rotation_Subindex.z, WorldPosition.xyz, OldWorldPosition.xyz, Right, Up, false);
    
    float2 Size = abs(IN.Size_Rotation_Subindex.xy);
    WorldPosition.xyz += Size.x * (IN.Position.x - 0.5f) * Right;
    WorldPosition.xyz += Size.y * (IN.Position.y - 0.5f) * Up;
    
    float3x3 TangentToWorld = ParticleSpriteCalcTangentBasis(Right, Up);
    
    OUT.Position = mul(View.WorldToProjection, WorldPosition);
    OUT.Color = float4(IN.ParticleColor.rgb, 1.0f);
    OUT.UV = IN.Position;
    OUT.TangentToWorld = TangentToWorld;
    
    return OUT;
}

// -------------------------------------------------------------------------------------

struct PixelShaderInput
{
    float4 Color : COLOR;
    float2 UV : UV;
    float3x3 TangentToWorld : TBN;
};

PixelShaderOutput Main_PS(PixelShaderInput IN) : SV_Target
{
    ConstantBuffer<ParticleSpriteBuffer> ParticleBuffer = ResourceDescriptorHeap[BindlessResources.PrimitiveIndex];
    ConstantBuffer<ParametersBuffers> Parameters = ResourceDescriptorHeap[BindlessResources.ParametersBufferIndex];
    
    Texture2D BaseColorTexture = ResourceDescriptorHeap[Parameters.BaseColor_Texture];
    SamplerState BaseColorSampler = ResourceDescriptorHeap[Parameters.BaseColor_Sampler];
    
    Texture2D NormalTexture = ResourceDescriptorHeap[Parameters.Normal_Texture];
    SamplerState NormalSampler = ResourceDescriptorHeap[Parameters.Normal_Sampler];
    
    Texture2D MasksTexture = ResourceDescriptorHeap[Parameters.Masks_Texture];
    SamplerState MasksSampler = ResourceDescriptorHeap[Parameters.Masks_Sampler];
    
    float3 BaseColor = BaseColorTexture.Sample(BaseColorSampler, IN.UV).xyz;
    float3 Masks = MasksTexture.Sample(MasksSampler, IN.UV).xyz;
    float3 Normal = NormalTexture.Sample(NormalSampler, IN.UV).rgb;
    Normal = ReconstructTextureNormal(Normal.xy);
    Normal = normalize(mul(Normal, IN.TangentToWorld));
    //Normal = IN.TangentToWorld[1];
    float2 N = EncodeNormal(Normal);
    
    float4 OutColorDeferred = float4(0.0, 0.0, 0.0, 1);
    float4 OutBaseColor = float4(BaseColor, 1);
    float2 OutWorldNormal = N;
    float4 OutMasks = float4(Masks, Uint8ToFloat(SHADING_MODEL_LIT));
    
    //OutBaseColor.xy = IN.UV;
    OutBaseColor.xyz = RadialGradientExponential(IN.UV).xxx;
    
    PixelShaderOutput OUT;
    
#if MAIN_PASS
    OUT.ColorDeferred = OutColorDeferred;
    OUT.BaseColor = OutBaseColor;
    OUT.WorldNormal = OutWorldNormal;
    OUT.Masks = OutMasks;
#elif HITPROXY_PASS
    OUT.Guid = ParticleBuffer.Guid;
#endif
    
    return OUT;
}