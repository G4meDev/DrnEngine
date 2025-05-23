#include "Common.hlsl"

// SUPPORT_MAIN_PASS
// SUPPORT_HIT_PROXY_PASS
// SUPPORT_EDITOR_SELECTION_PASS

ConstantBuffer<ViewBuffer> View : register(b0);

Texture2D BaseColorTexture : register(t0);
SamplerState BaseColorSampler : register(s0);

Texture2D NormalTexture : register(t1);
SamplerState NormalSampler : register(s1);

Texture2D MasksTexture : register(t2);
SamplerState MasksSampler : register(s2);

struct VertexShaderOutput
{
    float4 Color : COLOR;
    float3 Normal : NORMAL;
    float3x3 TBN : NORMAL1;
    float2 UV : TEXCOORD;
    float4 Position : SV_Position;
};

VertexShaderOutput Main_VS(VertexInputStaticMesh IN)
{
    VertexShaderOutput OUT;

    float3x3 TBN = float3x3(IN.Tangent, IN.Bitangent, IN.Normal);

    float3 VertexNormal = mul(View.LocalToWorld, float4(IN.Normal, 0.0f));
    float3 VertexTangent = mul(View.LocalToWorld, float4(IN.Tangent, 0.0f));
    float3 VertexBiNormal = mul(View.LocalToWorld, float4(IN.Bitangent, 0.0f));

    OUT.TBN = float3x3(VertexTangent, VertexBiNormal, VertexNormal);
    
    OUT.Position = mul(View.LocalToProjection, float4(IN.Position, 1.0f));
    OUT.Color = float4(IN.Color, 1.0f);
    OUT.Normal = IN.Normal;
    OUT.UV = IN.UV1;
    
    return OUT;
}

// -------------------------------------------------------------------------------------

struct PixelShaderInput
{
    float4 Color : COLOR;
    float3 Normal : NORMAL;
    float3x3 TBN : NORMAL1;
    float2 UV : TEXCOORD;
};

PixelShaderOutput Main_PS(PixelShaderInput IN) : SV_Target
{
    PixelShaderOutput OUT;
 

#if MAIN_PASS
    float3 BaseColor = BaseColorTexture.Sample(BaseColorSampler, IN.UV).xyz;
    float3 Masks = MasksTexture.Sample(MasksSampler, IN.UV).xyz;
    
    float3 Normal = NormalTexture.Sample(NormalSampler, IN.UV).xyz;
    Normal = Normal * 2 - 1;
    Normal = Normal.rbg;
    Normal = normalize(mul(IN.TBN, Normal));
    float3 WorldNormal = EncodeNormal(IN.Normal);
    
    OUT.ColorDeferred = float4(0.0, 0.0, 0.0, 1);
    OUT.BaseColor = float4(BaseColor, 1);
    OUT.WorldNormal = float4( WorldNormal, 1);
    OUT.Masks = float4(Masks, 1);
#elif HitProxyPass
    OUT.Guid = View.Guid;
#endif
    
    return OUT;
}