//#define STATICMESH 1
#include "Common.hlsl"

// DOMAIN_DECAL

// SUPPORT_DECAL
// SUPPORT_STATICMESH

// SUPPORT_DECAL_PASS

ConstantBuffer<DecalResources> BindlessResources : register(b0);

struct ParametersBuffers
{
    VECTOR(TintColor, TintColor)
    
    SCALAR(TintIntensity, TintIntensity)
    SCALAR(RoughnessIntensity, RoughnessIntensity)
    SCALAR(NormalIntensity, NormalIntensity)

    TEX2D(BaseColor, BaseColorTexture)
    TEX2D(Normal, NormalTexture)
    TEX2D(Masks, MasksTexture)
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
    
#if DECAL
    ConstantBuffer<DecalBuffer> Decal = ResourceDescriptorHeap[BindlessResources.DecalBufferIndex];
    OUT.Position = mul(Decal.LocalToProjection, float4(IN.Position, 1.0f));
#elif STATICMESH
    ConstantBuffer<ViewBuffer> View = ResourceDescriptorHeap[BindlessResources.ViewIndex];
    ConstantBuffer<PrimitiveBuffer> Primitive = ResourceDescriptorHeap[BindlessResources.PrimitiveIndex];
    float4 WorldPosition = mul(Primitive.LocalToWorld, float4(IN.Position, 1.0f));
    OUT.Position = mul(View.WorldToProjection, WorldPosition);
    OUT.UV0 = IN.UV1;
#endif
    
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

#if DECAL

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

    Texture2D BaseColorTexture = ResourceDescriptorHeap[Parameters.BaseColor_Texture];
    SamplerState BaseColorSampler = ResourceDescriptorHeap[Parameters.BaseColor_Sampler];
    
    Texture2D NormalTexture = ResourceDescriptorHeap[Parameters.Normal_Texture];
    SamplerState NormalSampler = ResourceDescriptorHeap[Parameters.Normal_Sampler];
    
    Texture2D MasksTexture = ResourceDescriptorHeap[Parameters.Masks_Texture];
    SamplerState MasksSampler = ResourceDescriptorHeap[Parameters.Masks_Sampler];
    
    float2 DecalUVs = LocalPosition.xz * float2(0.5f, -0.5f) + 0.5f;
    
    float3 BaseColor = BaseColorTexture.Sample(BaseColorSampler, DecalUVs).xyz;
    float3 Normal = NormalTexture.Sample(NormalSampler, DecalUVs).xyz;
    float3 Masks = MasksTexture.Sample(MasksSampler, DecalUVs).xyz;
    
    float BlendAlpha = 1.0f;
    BlendAlpha *= 4 * (1 - abs(LocalPosition.y));
    BlendAlpha *= Masks.r;
    
    OUT.BaseColor = float4(BaseColor, BlendAlpha);
    OUT.Normal = float4(Normal, BlendAlpha);
    OUT.Masks = float4(0.0f, Masks.gb, BlendAlpha);

#elif STATICMESH

    ConstantBuffer<StaticSamplers> StaticSamplers = ResourceDescriptorHeap[BindlessResources.StaticSamplerBufferIndex];
    SamplerState LinearSampler = ResourceDescriptorHeap[StaticSamplers.LinearSamplerIndex];
    
    ConstantBuffer<ParametersBuffers> Parameters = ResourceDescriptorHeap[BindlessResources.ParametersBufferIndex];

    Texture2D BaseColorTexture = ResourceDescriptorHeap[Parameters.BaseColor_Texture];
    SamplerState BaseColorSampler = ResourceDescriptorHeap[Parameters.BaseColor_Sampler];
    
    Texture2D NormalTexture = ResourceDescriptorHeap[Parameters.Normal_Texture];
    SamplerState NormalSampler = ResourceDescriptorHeap[Parameters.Normal_Sampler];
    
    Texture2D MasksTexture = ResourceDescriptorHeap[Parameters.Masks_Texture];
    SamplerState MasksSampler = ResourceDescriptorHeap[Parameters.Masks_Sampler];
    
    float3 BaseColor = BaseColorTexture.Sample(BaseColorSampler, IN.UV0).xyz;
    float3 Normal = NormalTexture.Sample(NormalSampler, IN.UV0).xyz;
    float3 Masks = MasksTexture.Sample(MasksSampler, IN.UV0).xyz;
    
    float BlendAlpha = Masks.r;
    
    OUT.BaseColor = float4(BaseColor, BlendAlpha);
    OUT.Normal = float4(Normal, BlendAlpha);
    OUT.Masks = float4(0.0f, Masks.gb, BlendAlpha);

#endif
    
    return OUT;
}