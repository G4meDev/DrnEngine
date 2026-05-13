//#define STATICMESH 1
#include "Common.hlsl"

// DOMAIN_DECAL
// BLEND_TRANSLUCENT
// SHADING_LIT

// SUPPORT_DECAL

// SUPPORT_DECAL_PASS

ConstantBuffer<DecalResources> BindlessResources : register(b0);

struct ParametersBuffers
{
    SCALAR(Power, Power)
    SCALAR(Mul, Mul)
    SCALAR(Tiling, Tiling)
    
    TEX2D(Caustics, CausticsTexture)
    TEX2D(Distortion, DistortionTexture)
};

struct VertexShaderInput
{
    float3 Position : POSITION;

#if STATICMESH
    float2 UV1 : TEXCOORD0;
#endif
};

struct VertexShaderOutput
{
    float4 Position : SV_Position;
    
#if STATICMESH
    float2 UV0 : TEXCOORD0;
#endif
};

VertexShaderOutput Main_VS(VertexShaderInput IN)
{
    VertexShaderOutput OUT;
    
    ConstantBuffer<DecalBuffer> Decal = ResourceDescriptorHeap[BindlessResources.DecalBufferIndex];
    OUT.Position = mul(Decal.LocalToProjection, float4(IN.Position, 1.0f));
    
    return OUT;
}

//// -------------------------------------------------------------------------------------

struct PixelShaderInput
{
    float4 Position : SV_Position;
    
#if STATICMESH
    float2 UV0 : TEXCOORD0;
#endif
};

struct PixelShaderOutput
{
    float4 BaseColor : SV_TARGET0;
    float4 Normal : SV_TARGET1;
    float4 Masks : SV_TARGET2;
};

PixelShaderOutput Main_PS(PixelShaderInput IN) : SV_Target
{
    PixelShaderOutput OUT;

    ConstantBuffer<ViewBuffer> View = ResourceDescriptorHeap[BindlessResources.ViewIndex];
    ConstantBuffer<DecalBuffer> Decal = ResourceDescriptorHeap[BindlessResources.DecalBufferIndex];
    ConstantBuffer<StaticSamplers> StaticSamplers = ResourceDescriptorHeap[BindlessResources.StaticSamplerBufferIndex];
    SamplerState LinearSampler = ResourceDescriptorHeap[StaticSamplers.LinearSamplerIndex];
    SamplerState PointSampler = ResourceDescriptorHeap[StaticSamplers.PointSamplerIndex];
    
    Texture2D DepthTexture = ResourceDescriptorHeap[BindlessResources.DepthTexture];

    float2 ScreenUV = SvPositionToViewportUV(IN.Position.xy, View.InvSize);
    float PixelDepth = DepthTexture.Sample(PointSampler, ScreenUV).x;
    float4 ClipPosition = float4(ViewportUVToScreenPos(ScreenUV), PixelDepth, 1);
    
    float4 LocalPosition = mul(Decal.ProjectionToLocal, ClipPosition);
    LocalPosition.xyz /= LocalPosition.w;
    
    clip(LocalPosition.xyz + 1.0f);
    clip(1.0f - LocalPosition.xyz);
    
    ConstantBuffer<ParametersBuffers> Parameters = ResourceDescriptorHeap[BindlessResources.ParametersBufferIndex];

    Texture2D CausticsTexture = ResourceDescriptorHeap[Parameters.Caustics_Texture];
    SamplerState CausticsSampler = ResourceDescriptorHeap[Parameters.Caustics_Sampler];
    
    Texture2D DistortionTexture = ResourceDescriptorHeap[Parameters.Distortion_Texture];
    SamplerState DistortionSampler = ResourceDescriptorHeap[Parameters.Distortion_Sampler];

    float2 DecalUVs = LocalPosition.xz * float2(0.5f, -0.5f) + 0.5f;
    DecalUVs *= Parameters.Tiling;
    
    float CausticMask = CausticsTexture.Sample(CausticsSampler, DecalUVs).r;
    CausticMask = saturate(pow(CausticMask, Parameters.Power) * Parameters.Mul);
    float3 BaseColor = CausticMask.rrr;
    float3 Normal = float3(0, 1, 0);
    float3 Masks = float3(0, .2, 1);
    
    float BlendAlpha = 1.0f;
    BlendAlpha *= 4 * (1 - abs(LocalPosition.y));
    //BlendAlpha *= Masks.r;
    
    OUT.BaseColor = float4(BaseColor, BlendAlpha);
    OUT.Normal = float4(Normal, 0);
    OUT.Masks = float4(0.0f, Masks.gb, 0);
    
    return OUT;
}