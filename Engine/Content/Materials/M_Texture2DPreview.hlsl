#include "Common.hlsl"

// SUPPORT_EDITOR_PRIMITIVE_PASS

ConstantBuffer<ViewBuffer> View : register(b0);

Texture2D Texture : register(t0);
SamplerState TextureSampler : register(s0);

struct VertexShaderOutput
{
    float2 UV : TEXCOORD0;
    float4 Position : SV_Position;
};

VertexShaderOutput Main_VS(VertexInputStaticMesh IN)
{
    VertexShaderOutput OUT;

    OUT.Position = mul(View.LocalToProjection, float4(IN.Position, 1.0f));
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
    PixelShaderOutput OUT;
    OUT.Color = Texture.Sample(TextureSampler, IN.UV);

    return OUT;
}