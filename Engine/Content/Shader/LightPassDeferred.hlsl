struct VertexInputPosUV
{
    float3 Position : POSITION;
};

struct ViewBuffer
{
    matrix LocalToProjection;
};

ConstantBuffer<ViewBuffer> View : register(b0);

Texture2D BaseColorTexture : register(t0);
Texture2D WorldNormalTexture : register(t1);
Texture2D MasksTexture : register(t2);
Texture2D DepthTexture : register(t3);

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
    float4 BaseColor = BaseColorTexture.Sample(TextureSampler, float2(2, 2));
    float4 WorldNormal = WorldNormalTexture.Sample(TextureSampler, float2(2, 2));
    float4 Masks = MasksTexture.Sample(TextureSampler, float2(2, 2));
    float4 Depth = DepthTexture.Sample(TextureSampler, float2(2, 2));
    
    return BaseColor + WorldNormal + Masks + Depth;
    //return float4(0.5, 0, 0, 0.4);
}