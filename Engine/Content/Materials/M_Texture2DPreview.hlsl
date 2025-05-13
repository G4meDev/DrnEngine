struct ModelViewProjection
{
    matrix MVP;
};

ConstantBuffer<ModelViewProjection> ModelViewProjectionCB : register(b0);

Texture2D Texture : register(t0);
SamplerState TextureSampler : register(s0);

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
    float2 UV : TEXCOORD0;
    float4 Position : SV_Position;
};

VertexShaderOutput Main_VS(VertexInput IN)
{
    VertexShaderOutput OUT;

    OUT.Position = mul(ModelViewProjectionCB.MVP, float4(IN.Position, 1.0f));
    OUT.UV = IN.UV1;
    
    return OUT;
}

// -------------------------------------------------------------------------------------

struct PixelShaderInput
{
    float2 UV : TEXCOORD0;
};

float4 Main_PS(PixelShaderInput IN) : SV_Target
{
    return Texture.Sample(TextureSampler, IN.UV);
}