#include "Common.hlsl"

// DOMAIN_SURFACE
// BLEND_OPAQUE
// SHADING_LIT

// SUPPORT_STATICMESH

// SUPPORT_MAIN_PASS
// SUPPORT_HIT_PROXY_PASS
// SUPPORT_PRE_PASS
// SUPPORT_EDITOR_SELECTION_PASS

ConstantBuffer<StandardResources> BindlessResources : register(b0);

struct VertexShaderOutput
{
    float4 Color : COLOR;
    float4 Position : SV_Position;
};

struct PixelShaderOutput
{
#if MAIN_PASS
    float4 ColorDeferred : SV_TARGET0;
    float4 BaseColor : SV_TARGET1;
    float4 WorldNormal : SV_TARGET2;
    float4 Masks : SV_TARGET3;
    float4 MasksB : SV_TARGET4;
#elif HITPROXY_PASS
    uint4 Guid;
#elif EDITOR_PRIMITIVE_PASS
    float4 Color;
#endif
};

VertexShaderOutput Main_VS(VertexInput IN)
{
    VertexShaderOutput OUT;
    
    ConstantBuffer<ViewBuffer> View = ResourceDescriptorHeap[BindlessResources.ViewIndex];
    ConstantBuffer<PrimitiveBuffer> Primitive = ResourceDescriptorHeap[BindlessResources.PrimitiveIndex];
    
    float4 WorldPosition = mul(Primitive.LocalToWorld, float4(IN.Position, 1.0f));
    OUT.Position = mul(View.WorldToProjection, WorldPosition);
    OUT.Color = float4(IN.Color, 1.0f);

    return OUT;
}

// -------------------------------------------------------------------------------------

struct PixelShaderInput
{
    float4 Color : COLOR;
};

PixelShaderOutput Main_PS(PixelShaderInput IN) : SV_Target
{
    ConstantBuffer<PrimitiveBuffer> PrimitiveBuffer = ResourceDescriptorHeap[BindlessResources.PrimitiveIndex];

    PixelShaderOutput OUT;
    
#if MAIN_PASS
    OUT.ColorDeferred = IN.Color * 0.7f;
    OUT.BaseColor = float4(0.7, 0.5, 1, 1);
    OUT.WorldNormal = float4(0, 1, 0, 1);
    OUT.Masks = float4(0.2, 1, 0.4, 1);
#elif HITPROXY_PASS
    OUT.Guid = PrimitiveBuffer.Guid;
#endif
    
    return OUT;
}