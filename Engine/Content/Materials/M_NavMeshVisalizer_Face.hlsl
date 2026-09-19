#include "Common.hlsl"

// DOMAIN_SURFACE
// BLEND_TRANSLUCENT
// SHADING_UNLIT

// SUPPORT_STATICMESH

// SUPPORT_MAIN_PASS
// SUPPORT_HIT_PROXY_PASS
// TWO_SIDED

ConstantBuffer<StandardResources> BindlessResources : register(b0);

struct VertexShaderOutput
{
    float4 Position : SV_Position;
    float4 Color : COLOR;
};

struct PixelShaderOutput
{
#if TRANSLUCENCY_PASS
    float4 TranslucentColor;
#elif HITPROXY_PASS
    uint4 Guid;
#endif
};

VertexShaderOutput Main_VS(VertexInput IN)
{
    VertexShaderOutput OUT;
    
    ConstantBuffer<ViewBuffer> View = ResourceDescriptorHeap[BindlessResources.ViewIndex];
    ConstantBuffer<PrimitiveBuffer> Primitive = ResourceDescriptorHeap[BindlessResources.PrimitiveIndex];
    
    matrix LocalToWorld = Primitive.LocalToWorld;
    float4 WorldPosition = mul(LocalToWorld, float4(IN.Position, 1.0f));
    OUT.Position = mul(View.WorldToProjection, WorldPosition);
    
    OUT.Color = float4(IN.Color, 0);

    return OUT;
}

// -------------------------------------------------------------------------------------

struct PixelShaderInput
{
    float4 Position : SV_Position;
    float4 Color : COLOR;
};

PixelShaderOutput Main_PS(PixelShaderInput IN, bool bFrontFace : SV_IsFrontFace, uint ID : SV_PrimitiveID) : SV_Target
{
    ConstantBuffer<PrimitiveBuffer> Primitive = ResourceDescriptorHeap[BindlessResources.PrimitiveIndex];

    PixelShaderOutput OUT;
    
#if TRANSLUCENCY_PASS
    float3 BaseColorFront = float3(0, 1, 0);
    float3 BaseColoBack = float3(1, 0, 0);
    float3 SelectedColor = float3(1, 1, 0);
    
    float3 BaseColor = bFrontFace ? BaseColorFront : BaseColoBack;
    BaseColor = IN.Color.r > 0 ? SelectedColor : BaseColor;
    float Opacity = 0.4f;
    
    OUT.TranslucentColor = float4(BaseColor, Opacity);
#elif HITPROXY_PASS
    OUT.Guid = Primitive.Guid;
    OUT.Guid[2] = 1;
    OUT.Guid[3] = ID;
#endif
    
    return OUT;
}