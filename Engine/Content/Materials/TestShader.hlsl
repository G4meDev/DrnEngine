#include "Common.hlsl"

// SUPPORT_HIT_PROXY_PASS

ConstantBuffer<ViewBuffer> View : register(b0);

struct VertexShaderOutput
{
    float4 Color : COLOR;
    float4 Position : SV_Position;
};

VertexShaderOutput Main_VS(VertexInputStaticMesh IN)
{
    VertexShaderOutput OUT;

    OUT.Position = mul(View.LocalToProjection, float4(IN.Position, 1.0f));
    OUT.Color = float4(IN.Color, 1.0f);

    return OUT;
}

// -------------------------------------------------------------------------------------

struct PixelShaderInput
{
    float4 Color : COLOR;
};

#if HitProxyPass
uint4 Main_PS(PixelShaderInput IN) : SV_Target
#else
BasePassPixelShaderOutput Main_PS(PixelShaderInput IN) : SV_Target
#endif
{
    BasePassPixelShaderOutput OUT;
 
    OUT.ColorDeferred = float4(1, 1, 1, 1);
    OUT.BaseColor = float4(0.7, 0.5, 1, 1);
    OUT.WorldNormal = float4(0, 1, 0, 1);
    OUT.Masks = float4(0.2, 1, 0.4, 1);
    
#if HitProxyPass
    return View.Guid;
#else
    return OUT;
#endif
}