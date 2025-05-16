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

//Texture2D ResolveTexture : register(t0);
//SamplerState ResolveSampler : register(s0);

struct VertexShaderOutput
{
    float2 UV : TEXCOORD0;
    float4 Position : SV_Position;
};

VertexShaderOutput Main_VS(VertexInputPosUV IN)
{
    VertexShaderOutput OUT;

    OUT.Position = mul(View.LocalToView, float4(IN.Position, 1.0f));
    OUT.UV = IN.UV;
    
    return OUT;
}

struct PixelShaderInput
{
    float2 UV : TEXCOORD0;
};

float4 Main_PS(PixelShaderInput IN) : SV_Target
{
    //return ResolveTexture.Sample(ResolveSampler, IN.UV);
    float Alpha = 0.6f;
    return Alpha.xxxx;
}