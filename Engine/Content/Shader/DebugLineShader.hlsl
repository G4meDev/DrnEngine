#include "../Materials/Common.hlsl"

ConstantBuffer<ViewBuffer> View : register(b0);

struct VertexShaderOutput
{
	float4 Color : COLOR;
	float Thickness : THICKNESS;
	float4 Position : SV_Position;
};

VertexShaderOutput Main_VS(VertexInputPosColor IN)
{
	VertexShaderOutput OUT;

	OUT.Position = mul(View.LocalToProjection, float4(IN.Position, 1.0f));
	OUT.Color = float4(IN.Color.xyz, 1.0f);
	OUT.Thickness = IN.Color.a;

	return OUT;
}

// -------------------------------------------------------------------------------------

struct PixelShaderInput
{
	float4 Color : COLOR;
};

float4 Main_PS(PixelShaderInput IN) : SV_Target
{
    return IN.Color;
}