struct ModelViewProjection
{
    matrix MVP;
};

ConstantBuffer<ModelViewProjection> ModelViewProjectionCB : register(b0);

Texture2D TestTexture : register(t0);
SamplerState TestSampler : register(s0);

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
    float4 Color : COLOR;
    float2 UV : TEXCOORD0;
    float4 Position : SV_Position;
};

VertexShaderOutput Main_VS(VertexInput IN)
{
    VertexShaderOutput OUT;

    OUT.Position = mul(ModelViewProjectionCB.MVP, float4(IN.Position, 1.0f));
    OUT.Color = float4(IN.UV1, 0.0f, 1.0f);
    OUT.UV = IN.UV1;
    
    return OUT;
}

// -------------------------------------------------------------------------------------

struct PixelShaderInput
{
    float4 Color : COLOR;
    float2 UV : TEXCOORD0;
};

float4 Main_PS(PixelShaderInput IN) : SV_Target
{
    //return float4(IN.UV, 0, 1);
    return TestTexture.Sample(TestSampler, IN.UV);
}