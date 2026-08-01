#include "Common.hlsl"

// DOMAIN_SURFACE
// BLEND_OPAQUE
// SHADING_LIT

// SUPPORT_STATICMESH

// SUPPORT_MAIN_PASS
// SUPPORT_PRE_PASS

ConstantBuffer<StandardResources> BindlessResources : register(b0);

struct ParametersBuffers
{
    VECTOR(ShowColor, ShowColor)
    SCALAR(MipLevel, MipLevel)
    TEX2D(BaseColor, Texture)
};

struct VertexShaderOutput
{
    float2 UV : TEXCOORD0;
    float3 WorldPosition: WORLDPOS;
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

VertexShaderOutput Main_VS(VertexInputStaticMesh IN)
{
    VertexShaderOutput OUT;
    
    ConstantBuffer<ViewBuffer> View = ResourceDescriptorHeap[BindlessResources.ViewIndex];
    ConstantBuffer<PrimitiveBuffer> Primitive = ResourceDescriptorHeap[BindlessResources.PrimitiveIndex];

    float4 WorldPosition = mul(Primitive.LocalToWorld, float4(IN.Position, 1.0f));
    OUT.WorldPosition = WorldPosition.xyz;
    OUT.Position = mul(View.WorldToProjection, WorldPosition);
    OUT.UV = IN.UV1;
    
    return OUT;
}

// -------------------------------------------------------------------------------------

struct PixelShaderInput
{
    float2 UV : TEXCOORD0;
    float3 WorldPosition : WORLDPOS;
};

PixelShaderOutput Main_PS(PixelShaderInput IN) : SV_Target
{
    ConstantBuffer<StaticSamplers> StaticSamplers = ResourceDescriptorHeap[BindlessResources.StaticSamplerBufferIndex];
    SamplerState LinearSampler = ResourceDescriptorHeap[StaticSamplers.LinearSamplerIndex];
    
    ConstantBuffer<ParametersBuffers> Parameters = ResourceDescriptorHeap[BindlessResources.ParametersBufferIndex];

    Texture2D Texture = ResourceDescriptorHeap[Parameters.BaseColor_Texture];
    SamplerState Sampler = ResourceDescriptorHeap[Parameters.BaseColor_Sampler];
    
    PixelShaderOutput OUT;
    float2 UV = float2(1 - IN.UV.x, IN.UV.y);
    float4 Sample = Texture.SampleLevel(Sampler, UV, Parameters.MipLevel);
    
    //OUT.ColorDeferred = float4(BaseColor, 1);
    //OUT.ColorDeferred = pow(OUT.ColorDeferred, 1.0f / 2.2f);
    OUT.BaseColor = 0;
    OUT.WorldNormal = 0;
    OUT.Masks = 0;
    //OUT.Masks.a = 1.0f/255;
    OUT.Masks.a = 0;
    
    float3 Color;
    
    float4 ChannelMasks = step(0.1, Parameters.ShowColor);
    float SumMask = ChannelMasks.r + ChannelMasks.g + ChannelMasks.b + ChannelMasks.a;

    Color = Sample.rgb * ChannelMasks.rgb;
    if(SumMask == 1) // single channel view
    {
        float4 Mult = Sample.rgba * ChannelMasks.rgba;
        float MultAdd = Mult.r + Mult.g + Mult.b + Mult.a;
        Color = MultAdd;
    }
    
    else if (ChannelMasks.a > 0)
    {
        float PatternStepSize = 0.1f;
        float2 Pattern = floor((IN.WorldPosition.xy) / PatternStepSize);
        float Sum = Pattern.x + Pattern.y;
        float Mask = fmod(Sum, 2) == 0;
        float3 Background = lerp(0.3f, 0.8f, Mask);
        Color = lerp(Background, Color, Sample.a);
    }
    
    OUT.ColorDeferred = float4(Color, 1);
    
    return OUT;
}