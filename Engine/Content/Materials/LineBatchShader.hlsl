struct ModelViewProjection
{
    matrix MVP;
};

ConstantBuffer<ModelViewProjection> ModelViewProjectionCB : register(b0);

struct VertexPosColor
{
    float3 Position : POSITION;
    float4 Color : COLOR;
};

struct VertexShaderOutput
{
    float4 Color : COLOR;
    float Thickness : THICKNESS;
    float4 Position : SV_Position;
};

VertexShaderOutput Main_VS(VertexPosColor IN)
{
    VertexShaderOutput OUT;

    OUT.Position = mul(ModelViewProjectionCB.MVP, float4(IN.Position, 1.0f));
    OUT.Color = float4(IN.Color.xyz, 1.0f);
    OUT.Thickness = IN.Color.a;
    
    return OUT;
}

// -------------------------------------------------------------------------------------

struct GeometeryShaderOutput
{
    float4 Color : COLOR;
    float4 Position : SV_Position;
};

[maxvertexcount(6)]
void Main_GS(line VertexShaderOutput input[2], inout TriangleStream<GeometeryShaderOutput> OutputStream)
{
    //float Thickness = 0.1f;
    float Thickness = input[0].Thickness;
    
    float2 tan = normalize((input[1].Position - input[0].Position).xy);
    float4 Dir = float4(-tan.y, tan.x, 0.0f, 0.0f);
    
    GeometeryShaderOutput V1;
    V1.Color = input[0].Color;
    V1.Position = input[0].Position - Dir * Thickness;
    
    GeometeryShaderOutput V2;
    V2.Color = input[0].Color;
    V2.Position = input[0].Position + Dir * Thickness;

    GeometeryShaderOutput V3;
    V3.Color = input[1].Color;
    V3.Position = input[1].Position + Dir * Thickness;
    
    GeometeryShaderOutput V4;
    V4.Color = input[1].Color;
    V4.Position = input[1].Position - Dir * Thickness;
    
    OutputStream.Append(V1);
    OutputStream.Append(V2);
    OutputStream.Append(V3);
    
    OutputStream.RestartStrip();
    
    OutputStream.Append(V3);
    OutputStream.Append(V4);
    OutputStream.Append(V1);
}

// -------------------------------------------------------------------------------------

struct PixelShaderInput
{
    float4 Color : COLOR;
};

float4 Main_PS(PixelShaderInput IN) : SV_Target
{
    //return pow(abs(IN.Color), 1.0f / 2.2f);
    return IN.Color;
}