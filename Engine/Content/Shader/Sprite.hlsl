struct VertexInputPosUV
{
    float3 Position : POSITION;
    float2 UV : TEXCOORD;
};

struct ViewBuffer
{
    matrix LocalToProjection;
    uint4 Guid;
};

ConstantBuffer<ViewBuffer> View : register(b0);

Texture2D SpriteImage : register(t0);
SamplerState TextureSampler : register(s0);

struct VertexShaderOutput
{
    float2 UV : TEXCOORD0;
    float4 Position : SV_Position;
};

VertexShaderOutput Main_VS(VertexInputPosUV IN)
{
    VertexShaderOutput OUT;

    OUT.Position = mul(View.LocalToProjection, float4(IN.Position, 1.0f));
    OUT.UV = IN.UV;

    return OUT;
}

struct PixelShaderInput
{
    float2 UV : TEXCOORD0;
};

#if HitProxyPass
uint4 Main_PS(PixelShaderInput IN) : SV_Target
#else
float4 Main_PS(PixelShaderInput IN) : SV_Target
#endif
{
    float4 SpriteColor = SpriteImage.Sample(TextureSampler, IN.UV);
    clip(SpriteColor.a < 0.002 ? -1 : 1);

#if HitProxyPass
    return View.Guid;
#else
    return SpriteColor;
#endif
}