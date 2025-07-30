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

	OUT.Position = float4(IN.Position, 1);
	OUT.Color = float4(IN.Color.xyz, 1.0f);
	OUT.Thickness = IN.Thickness;
	
	return OUT;
}

// -------------------------------------------------------------------------------------

struct GeometeryShaderOutput
{
	float4 Color : COLOR;
	float4 Position : SV_Position;
};

[maxvertexcount(24)]
void Main_GS(line VertexShaderOutput input[2], inout TriangleStream<GeometeryShaderOutput> OutputStream)
{
    ConstantBuffer<ViewBuffer> View = ResourceDescriptorHeap[BindlessResources.ViewBufferIndex];
    
	float Thickness = input[0].Thickness * 255;

	GeometeryShaderOutput V1;
	GeometeryShaderOutput V2;
	GeometeryShaderOutput V3;
	GeometeryShaderOutput V4;
	GeometeryShaderOutput V5;
	GeometeryShaderOutput V6;
	GeometeryShaderOutput V7;
	GeometeryShaderOutput V8;
    V1.Color = V2.Color = V3.Color = V4.Color = V5.Color = V6.Color = V7.Color = V8.Color = input[0].Color;
	
    float3 Tan		= normalize(input[1].Position.xyz - input[0].Position.xyz);
	float3 Right;
    if (Tan.x == 0 && (Tan.y == 1 || Tan.y == -1) && Tan.z == 0)
        Right = float3(0, 0, 1);
	else
		Right = normalize(cross(Tan, float3(0, 1, 0)));
	
	float3 Up		= normalize(cross(Right, Tan));
	
    float3 StartPos = input[0].Position.xyz - Tan * Thickness * 0.5f;
    float3 EndPos = input[1].Position.xyz + Tan * Thickness * 0.5f;

    float3 Pos;
	
    Pos = StartPos + Up * Thickness * 0.5f;
	V1.Position = mul(View.WorldToProjection, float4(Pos, 1.0f));
	
    Pos = StartPos + Right * Thickness * 0.5f;
    V2.Position = mul(View.WorldToProjection, float4(Pos, 1.0f));

    Pos = StartPos - Up * Thickness * 0.5f;
    V3.Position = mul(View.WorldToProjection, float4(Pos, 1.0f));
	
    Pos = StartPos - Right * Thickness * 0.5f;
    V4.Position = mul(View.WorldToProjection, float4(Pos, 1.0f));
	
    Pos = EndPos + Up * Thickness * 0.5f;
    V5.Position = mul(View.WorldToProjection, float4(Pos, 1.0f));

    Pos = EndPos + Right * Thickness * 0.5f;
    V6.Position = mul(View.WorldToProjection, float4(Pos, 1.0f));
	
    Pos = EndPos - Up * Thickness * 0.5f;
    V7.Position = mul(View.WorldToProjection, float4(Pos, 1.0f));
	
    Pos = EndPos - Right * Thickness * 0.5f;
    V8.Position = mul(View.WorldToProjection, float4(Pos, 1.0f));

	OutputStream.Append(V1);
	OutputStream.Append(V2);
	OutputStream.Append(V6);
		
	OutputStream.RestartStrip();
	
	OutputStream.Append(V6);
	OutputStream.Append(V5);
	OutputStream.Append(V1);
	
    OutputStream.RestartStrip();
	
    OutputStream.Append(V2);
    OutputStream.Append(V3);
    OutputStream.Append(V7);
	
    OutputStream.RestartStrip();
	
    OutputStream.Append(V7);
    OutputStream.Append(V6);
    OutputStream.Append(V2);
	
    OutputStream.RestartStrip();
	
    OutputStream.Append(V3);
    OutputStream.Append(V4);
    OutputStream.Append(V8);
	
    OutputStream.RestartStrip();
	
    OutputStream.Append(V8);
    OutputStream.Append(V7);
    OutputStream.Append(V3);
	
    OutputStream.RestartStrip();
	
    OutputStream.Append(V4);
    OutputStream.Append(V1);
    OutputStream.Append(V5);
	
    OutputStream.RestartStrip();
	
    OutputStream.Append(V5);
    OutputStream.Append(V8);
    OutputStream.Append(V4);
}

// -------------------------------------------------------------------------------------

struct PixelShaderInput
{
	float4 Color : COLOR;
};

float4 Main_PS(PixelShaderInput IN) : SV_Target
{
    float3 r = IN.Color.rgb + float3(0.5, 0.5, 0.5);
	
    return float4(r, 1);
    //return float4(IN.Color.rgb, 1);
}