struct VertexInputPosUV
{
    float3 Position : POSITION;
    float2 UV : TEXCOORD;
};

struct ViewBuffer
{
    matrix LocalToView;
    uint2 RenderTargetSize;
};

ConstantBuffer<ViewBuffer> View : register(b0);

Texture2D HdrImage : register(t0);
SamplerState TextureSampler : register(s0);

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

    return OUT;
}

struct PixelShaderInput
{
    float2 UV : TEXCOORD0;
};

float4 Main_PS(PixelShaderInput IN) : SV_Target
{
    //uint2 ScreenPos = IN.UV * View.RenderTargetSize;

    float4 HdrColor = HdrImage.Sample(TextureSampler, IN.UV);
    HdrColor = HdrColor / 1 + HdrColor;
    
    float a = 1.0 / 2.2;
    HdrColor.xyz = pow(HdrColor.xyz, a.xxx);
    return HdrColor;
}