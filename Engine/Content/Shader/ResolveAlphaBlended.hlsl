struct VertexInputPosUV
{
    float3 Position : POSITION;
    float2 UV : TEXCOORD;
};

struct ViewBuffer
{
    matrix LocalToView;
};

ConstantBuffer<ViewBuffer> View : register(b0);

Texture2D ResolveTexture : register(t0);
SamplerState ResolveSampler : register(s0);

struct VertexShaderOutput
{
    float2 UV : TEXCOORD0;
    float4 Position : SV_Position;
};

VertexShaderOutput Main_VS(VertexInputPosUV IN)
{
    VertexShaderOutput OUT;

    OUT.Position = mul(View.LocalToView, float4(IN.Position, 1.0f));
    OUT.Position.z = 0;
    OUT.UV = IN.UV;
    //OUT.UV = IN.UV * 2;

    return OUT;
}

struct PixelShaderInput
{
    float2 UV : TEXCOORD0;
};

float4 Main_PS(PixelShaderInput IN) : SV_Target
{
    float4 Texture = ResolveTexture.Sample(ResolveSampler, IN.UV);
    //float Alpha = 0.0f;
    //return float4(Alpha.xxx, 0.2f);
    
    return Texture;
}