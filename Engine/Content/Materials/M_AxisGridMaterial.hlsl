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
    
    float2 Frac = frac(Pos_xz);    
    float2 FracEdge = abs(Frac - 0.5f) - 0.45f;
    float Alpha = max(FracEdge.x, FracEdge.y) * 4;
    
    OUT.Color = Alpha.xxxx;
    //OUT.Color = float4(Alpha.xxx, 0.5f);
    return OUT;
}