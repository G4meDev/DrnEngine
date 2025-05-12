struct ModelViewProjection
{
    matrix MVP;
};

ConstantBuffer<ModelViewProjection> ModelViewProjectionCB : register(b0);

cbuffer MaterialScalars : register(b1)
{
    //Scalar Alpha
    float Alpha;
    //Scalar Rand
    float Rand;
    //Scalar WERWER
    float WERWER;
};

Texture2D TestTexture : register(t0);
SamplerState TestSampler : register(s0);

Texture2D TestTexture_2 : register(t1);
SamplerState TestSampler_2 : register(s1);

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
    float A = clamp(Alpha, 0, 1);
    
    float4 Texture1 = TestTexture.Sample(TestSampler, IN.UV);
    float4 Texture2 = TestTexture_2.Sample(TestSampler_2, IN.UV);
    return lerp(Texture1, Texture2, float4(A, A, A, 1.0f));
    //return lerp(Texture1, Texture2, float4(0.5f, 0.5f, 0.5f, 1.0f));
    //return Texture1;
}