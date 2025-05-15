#include "Common.hlsl"

ConstantBuffer<ViewBuffer> View : register(b0);

struct VertexShaderOutput
{
    float4 Color : COLOR;
    float4 Position : SV_Position;
};

VertexShaderOutput Main_VS(StaticMeshVertexInput IN)
{
    VertexShaderOutput OUT;

    OUT.Position = mul(View.LocalToProjection, float4(IN.Position, 1.0f));
    OUT.Color = float4(IN.UV1, 0.0f, 1.0f);

    return OUT;
}

// -------------------------------------------------------------------------------------

struct PixelShaderInput
{
    float4 Color : COLOR;
};

struct PixelShaderOutput
{
    float4 Color : SV_TARGET0;
    uint4 Guid : SV_TARGET1;
};

PixelShaderOutput Main_PS(PixelShaderInput IN) : SV_Target
{
    PixelShaderOutput OUT;
    OUT.Color = IN.Color * 1.0f;
    
    OUT.Guid = View.Guid;
    return OUT;
}