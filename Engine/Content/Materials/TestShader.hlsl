#include "Common.hlsl"

// SUPPORT_MAIN_PASS
// SUPPORT_HIT_PROXY_PASS
// SUPPORT_EDITOR_SELECTION_PASS

ConstantBuffer<ViewBuffer> View : register(b0);

struct VertexShaderOutput
{
    float4 Color : COLOR;
    float3 Normal : NORMAL;
    float4 Position : SV_Position;
};

VertexShaderOutput Main_VS(VertexInputStaticMesh IN)
{
    VertexShaderOutput OUT;

    OUT.Position = mul(View.LocalToProjection, float4(IN.Position, 1.0f));
    OUT.Color = float4(IN.Color, 1.0f);
    OUT.Normal = IN.Normal;

    return OUT;
}

// -------------------------------------------------------------------------------------

struct PixelShaderInput
{
    float4 Color : COLOR;
    float3 Normal : NORMAL;
};

PixelShaderOutput Main_PS(PixelShaderInput IN) : SV_Target
{
    PixelShaderOutput OUT;
 
#if MAIN_PASS
    OUT.ColorDeferred = float4(0.7, 0.4, 0.6, 1);
    OUT.BaseColor = float4(0.7, 0.5, 1, 1);
    OUT.WorldNormal = float4(IN.Normal, 1);
    OUT.Masks = float4(0.2, 1, 0.4, 1);
#elif HitProxyPass
    OUT.Guid = View.Guid;
#endif
    
    return OUT;
}