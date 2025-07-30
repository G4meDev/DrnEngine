//#include "../Materials/Common.hlsl"

struct Resources
{
    uint ViewBufferIndex;
};

ConstantBuffer<Resources> BindlessResources : register(b0);

struct ViewBuffer
{
    matrix WorldToView;
    matrix ViewToProjection;
    matrix WorldToProjection;
    matrix ProjectionToView;
    matrix ProjectionToWorld;
    matrix LocalToCameraView;

    uint2 RenderSize;
};

struct VertexInput
{
    float3 Position : POSITION;
    float4 Color : COLOR;
    float Thickness : THICKNESS;
};

struct VertexShaderOutput
{
	float4 Color : COLOR;
	float Thickness : THICKNESS;
	float4 Position : SV_Position;
};

VertexShaderOutput Main_VS(VertexInput IN)
{
	VertexShaderOutput OUT;

    ConstantBuffer<ViewBuffer> View = ResourceDescriptorHeap[BindlessResources.ViewBufferIndex];
    
	OUT.Position = mul(View.WorldToProjection, float4(IN.Position, 1.0f));
	OUT.Color = float4(IN.Color.xyz, 1.0f);
	OUT.Thickness = IN.Thickness;

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