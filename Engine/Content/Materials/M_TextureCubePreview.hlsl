#include "Common.hlsl"

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
    ConstantBuffer<PrimitiveBuffer> PrimitiveBuffer = ResourceDescriptorHeap[BindlessResources.PrimitiveIndex];
    
    VertexShaderOutput OUT;

    OUT.Position = mul(PrimitiveBuffer.LocalToProjection, float4(IN.Position, 1.0f));
    OUT.UV = IN.UV1;
    
    return OUT;
}

// -------------------------------------------------------------------------------------

struct PixelShaderInput
{
    float2 UV : TEXCOORD0;
};

PixelShaderOutput Main_PS(PixelShaderInput IN) : SV_Target
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
    //OUT.ColorDeferred = pow(OUT.ColorDeferred, 1.0f / 2.2f);
    OUT.BaseColor = 0;
    OUT.WorldNormal = 0;
    OUT.Masks = 0;
    //OUT.Masks.a = 1.0f/255;
    OUT.Masks.a = 0;
    
    return OUT;
}