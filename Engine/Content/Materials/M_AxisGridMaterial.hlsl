#include "Common.hlsl"

ConstantBuffer<ViewBuffer> View : register(b0);

struct VertexShaderOutput
{
    float3 WorldPos : WORLD_POS;
    float4 Position : SV_Position;
};

VertexShaderOutput Main_VS(VertexInputStaticMesh IN)
{
    VertexShaderOutput OUT;

    OUT.Position = mul(View.LocalToProjection, float4(IN.Position, 1.0f));
    OUT.WorldPos = mul(View.LocalToWorld, float4(IN.Position, 1.0f));

    return OUT;
}

// -------------------------------------------------------------------------------------

struct PixelShaderInput
{
    float3 WorldPos : WORLD_POS;
};

struct PixelShaderOutput
{
    float4 Color : SV_TARGET0;
};

PixelShaderOutput Main_PS(PixelShaderInput IN) : SV_Target
{
    PixelShaderOutput OUT;

    float2 Pos_xz = IN.WorldPos.xz;

    float2 Frac1 = frac(Pos_xz);
    float2 FracEdge1 = abs(Frac1 - 0.5f) - 0.47f;
    float Alpha1 = max(FracEdge1.x, FracEdge1.y) * 4;

    float2 Frac2 = frac(Pos_xz / 10);
    float2 FracEdge2 = abs(Frac2 - 0.5f) - 0.49f;
    float Alpha2 = max(FracEdge2.x, FracEdge2.y) * 20;

    float Alpha = max(Alpha1, Alpha2);
    
    OUT.Color = Alpha.xxxx;
    return OUT;
}