#include "../Materials/Common.hlsl"

struct VertexInputPosUV
{
    float3 Position : POSITION;
};

struct LightConstantBuffer
{
    matrix LocalToProjection;
    float3 Position;
    float Radius;
    float3 Color;
    float InvRadius;
};

ConstantBuffer<LightConstantBuffer> CB : register(b0);

Texture2D BaseColorTexture : register(t0);
Texture2D WorldNormalTexture : register(t1);
Texture2D MasksTexture : register(t2);
Texture2D DepthTexture : register(t3);

SamplerState TextureSampler : register(s0);

struct VertexShaderOutput
{
    float2 UV : TEXCOORD;
    float4 Position : SV_Position;
};

VertexShaderOutput Main_VS(VertexInputPosUV IN)
{
    VertexShaderOutput OUT;

    OUT.Position = mul(CB.LocalToProjection, float4(IN.Position, 1.0f));
    OUT.UV = VSPosToScreenUV(OUT.Position);

    return OUT;
}

struct PixelShaderInput
{
    float2 UV : TEXCOORD;
};

float4 Main_PS(PixelShaderInput IN) : SV_Target
{
    float4 BaseColor = BaseColorTexture.Sample(TextureSampler, IN.UV);
    float4 WorldNormal = WorldNormalTexture.Sample(TextureSampler, IN.UV);
    float4 Masks = MasksTexture.Sample(TextureSampler, IN.UV);
    float4 Depth = DepthTexture.Sample(TextureSampler, IN.UV);
    
    
    
    //return float4(CB.Color, 1);
    return Depth.xxxx;
    
    //return BaseColor + WorldNormal + Masks + Depth;
    //return WorldNormal;
}