struct ModelViewProjection
{
    matrix MVP;
    matrix ModelToWorld;
    uint4 Guid;
};

ConstantBuffer<ModelViewProjection> ModelViewProjectionCB : register(b0);

struct VertexInput
{
    float3 Position     : POSITION;
    float3 Color        : COLOR;
    float3 Normal       : NORMAL;
    float3 Tangent      : TANGENT;
    float3 Bitangent    : BINORMAL;
    float2 UV1          : TEXCOORD0;
    float2 UV2          : TEXCOORD1;
    float2 UV3          : TEXCOORD2;
    float2 UV4          : TEXCOORD3;
};

struct VertexShaderOutput
{
    float3 WorldPos : WORLD_POS;
    float4 Position : SV_Position;
};

VertexShaderOutput Main_VS(VertexInput IN)
{
    VertexShaderOutput OUT;

    OUT.Position = mul(ModelViewProjectionCB.MVP, float4(IN.Position, 1.0f));
    OUT.WorldPos = mul(ModelViewProjectionCB.ModelToWorld, float4(IN.Position, 1.0f));

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
    
    OUT.Color = float4( Alpha.xxx, 1.0f );
    return OUT;
}