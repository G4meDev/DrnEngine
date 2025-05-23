struct VertexInputPosUV
{
    float3 Position : POSITION;
};

struct ViewBuffer
{
    matrix LocalToProjection;
};

ConstantBuffer<ViewBuffer> View : register(b0);

Texture2D SpriteImage : register(t0);
SamplerState TextureSampler : register(s0);

struct VertexShaderOutput
{
    float4 Position : SV_Position;
};

VertexShaderOutput Main_VS(VertexInputPosUV IN)
{
    VertexShaderOutput OUT;

    OUT.Position = mul(View.LocalToProjection, float4(IN.Position, 1.0f));

    return OUT;
}

struct PixelShaderInput
{
};

float4 Main_PS(PixelShaderInput IN) : SV_Target
{
    return float4(1, 0, 0, 1);
}