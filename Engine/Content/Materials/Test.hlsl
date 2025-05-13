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

    //Scalar PAD
    float PAD;
    
// ----------------------------------------------

    //Vector4 LightDir
    float4 LightDir;
    
    //Vector4 ExampleVec4
    float4 ExampleVec4;
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
    float4 Color : COLOR0;
    float3 Normal : COLOR1;
    float2 UV : TEXCOORD0;
    float4 Position : SV_Position;
};

VertexShaderOutput Main_VS(VertexInput IN)
{
    VertexShaderOutput OUT;

    OUT.Position = mul(ModelViewProjectionCB.MVP, float4(IN.Position, 1.0f));
    OUT.Color = float4(IN.UV1, 0.0f, 1.0f);
    OUT.Normal = IN.Normal;
    OUT.UV = IN.UV1;
    
    return OUT;
}

// -------------------------------------------------------------------------------------

struct PixelShaderInput
{
    float4 Color : COLOR0;
    float3 Normal : COLOR1;
    float2 UV : TEXCOORD0;
};

struct PixelShaderOutput
{
    float4 Color : SV_TARGET0;
    uint4 Guid : SV_TARGET1;
};


PixelShaderOutput Main_PS(PixelShaderInput IN) : SV_Target
{
    PixelShaderOutput OUT;
    
    //float A = clamp(Alpha, 0, 1);
    //float4 B = float4(WERWER.xxx, 1);
    //float4 C = float4(ExampleVec4.xyz, 1);
    //
    float4 Texture1 = TestTexture.Sample(TestSampler, IN.UV);
    //float4 Texture2 = TestTexture_2.Sample(TestSampler_2, IN.UV);
    //return lerp(Texture1, Texture2, float4(A, A, A, 1.0f)) * B * C;

    float L = max(0, dot(IN.Normal, LightDir.xyz));
    
    OUT.Color = Texture1 * L * Alpha;
    OUT.Guid = uint4(33, 52, 61, 270);
    
    return OUT;
}