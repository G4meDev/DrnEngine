#include "Common.hlsl"

ConstantBuffer<ViewBuffer> View : register(b0);

Texture2D Texture : register(t0);
SamplerState TextureSampler : register(s0);

struct VertexShaderOutput
{
    float2 UV : TEXCOORD0;
    float4 Position : SV_Position;
};

VertexShaderOutput Main_VS(StaticMeshVertexInput IN)
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

struct PixelShaderOutput
{
    float4 Color : SV_TARGET0;
    uint4 Guid : SV_TARGET1;
};

PixelShaderOutput Main_PS(PixelShaderInput IN) : SV_Target
{
    PixelShaderOutput OUT;
    OUT.Color = Texture.Sample(TextureSampler, IN.UV);
    
    OUT.Guid = View.Guid;
    return OUT;
}