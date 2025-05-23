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
    float2 UV : TEXCOORD;
    float4 Position : SV_Position;
};

VertexShaderOutput Main_VS(VertexInputStaticMesh IN)
{
    VertexShaderOutput OUT;

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
    float2 UV : TEXCOORD;
};

PixelShaderOutput Main_PS(PixelShaderInput IN) : SV_Target
{
    PixelShaderOutput OUT;
 
#if MAIN_PASS
    float3 BaseColor = BaseColorTexture.Sample(BaseColorSampler, IN.UV).xyz;
    float3 Normal = NormalTexture.Sample(NormalSampler, IN.UV).xyz;
    float3 Masks = MasksTexture.Sample(MasksSampler, IN.UV).xyz;
    
    //float WorldNormal = EncodeNormal(IN.Normal);
    float3 WorldNormal = EncodeNormal(Normal);
    
    OUT.ColorDeferred = float4(0.0, 0.0, 0.0, 1);
    OUT.BaseColor = float4(BaseColor, 1);
    OUT.WorldNormal = float4( WorldNormal, 1);
    OUT.Masks = float4(Masks, 1);
#elif HitProxyPass
    OUT.Guid = View.Guid;
#endif
    
    return OUT;
}