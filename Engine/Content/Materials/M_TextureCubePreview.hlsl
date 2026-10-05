#include "Common.hlsl"

// DOMAIN_SURFACE
// BLEND_OPAQUE
// SHADING_UNLIT

// SUPPORT_STATICMESH

// SUPPORT_MAIN_PASS
// SUPPORT_PRE_PASS

ConstantBuffer<StandardResources> BindlessResources : register(b0);

struct ParametersBuffers
{
    SCALAR(MipLevel, MipLevel)
    TEXCUBE(BaseColor, Texture)
};

struct VertexShaderOutput
{
    float2 UV : TEXCOORD0;
    float4 Position : SV_Position;
};

struct PixelShaderOutput
{
#if MAIN_PASS
    float4 ColorDeferred : SV_TARGET0;
    float4 Masks : SV_TARGET3;
#elif HITPROXY_PASS
    uint4 Guid : SV_TARGET0;
#elif EDITOR_PRIMITIVE_PASS
    float4 Color : SV_TARGET0;
#endif
};

VertexShaderOutput Main_VS(VertexInputStaticMesh IN)
{
    VertexShaderOutput OUT;
    
    ConstantBuffer<ViewBuffer> View = ResourceDescriptorHeap[BindlessResources.ViewIndex];
    ConstantBuffer<PrimitiveBuffer> Primitive = ResourceDescriptorHeap[BindlessResources.PrimitiveIndex];

    float4 WorldPosition = mul(Primitive.LocalToWorld, float4(IN.Position, 1.0f));
    OUT.Position = mul(View.WorldToProjection, WorldPosition);
    OUT.UV = IN.UV1;
    
    return OUT;
}

// -------------------------------------------------------------------------------------

struct PixelShaderInput
{
    float2 UV : TEXCOORD0;
};

PixelShaderOutput Main_PS(PixelShaderInput IN)
{
    ConstantBuffer<StaticSamplers> StaticSamplers = ResourceDescriptorHeap[BindlessResources.StaticSamplerBufferIndex];
    SamplerState LinearSampler = ResourceDescriptorHeap[StaticSamplers.LinearSamplerIndex];
    
    ConstantBuffer<ParametersBuffers> Parameters = ResourceDescriptorHeap[BindlessResources.ParametersBufferIndex];

    TextureCube Texture = ResourceDescriptorHeap[Parameters.BaseColor_Texture];
    SamplerState Sampler = ResourceDescriptorHeap[Parameters.BaseColor_Sampler];
    
    const float CPI = 3.14159265;
    
    PixelShaderOutput OUT;
    
    float2 uv = IN.UV;
    
    float2 Angles = float2(2 * CPI * (uv.x + 0.5f), CPI * uv.y);
	
    float s = sin(Angles.y);
    float3 uvw = float3(s * sin(Angles.x), cos(Angles.y), -s * cos(Angles.x));
    
    float3 Sample = Texture.SampleLevel(Sampler, uvw, Parameters.MipLevel).rgb;
    
    OUT.ColorDeferred = float4(Sample, 1);
    OUT.Masks = float4(0, 0, 1, Uint8ToFloat(SHADING_MODEL_UNLIT));
    
    return OUT;
}