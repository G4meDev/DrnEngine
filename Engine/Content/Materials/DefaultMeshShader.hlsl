
//#include "../../../Engine/Content/Materials/Common.hlsl"

// SUPPORT_MAIN_PASS
// SUPPORT_HIT_PROXY_PASS
// SUPPORT_PRE_PASS
// SUPPORT_EDITOR_SELECTION_PASS


float3 EncodeNormal(float3 Normal)
{
    return Normal * 0.5 + 0.5;
}

struct Resources
{
    uint ViewIndex;
    uint PrimitiveIndex;
    uint StaticSamplerBufferIndex;
    uint TextureBufferIndex;
    uint ScalarBufferIndex;
    uint VectorBufferIndex;
};

ConstantBuffer<Resources> BindlessResources : register(b0);

struct View
{
    
};

struct Primitive
{
    matrix LocalToWorld;
    matrix LocalToProjection;
    uint4 Guid;
};

struct StaticSamplers
{
    uint LinearSamplerIndex;
};

struct VertexInputStaticMesh
{
    float3 Position : POSITION;
    float3 Color : COLOR;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 Bitangent : BINORMAL;
    float2 UV1 : TEXCOORD0;
    float2 UV2 : TEXCOORD1;
    float2 UV3 : TEXCOORD2;
    float2 UV4 : TEXCOORD3;
};

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
#elif HITPROXY_PASS
    uint4 Guid;
#elif EDITOR_PRIMITIVE_PASS
    float4 Color;
#endif
};

VertexShaderOutput Main_VS(VertexInputStaticMesh IN)
{
    ConstantBuffer<Primitive> PrimitiveBuffer = ResourceDescriptorHeap[BindlessResources.PrimitiveIndex];
    
    VertexShaderOutput OUT;

    OUT.Position = mul(PrimitiveBuffer.LocalToProjection, float4(IN.Position, 1.0f));
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
    ConstantBuffer<Primitive> PrimitiveBuffer = ResourceDescriptorHeap[BindlessResources.PrimitiveIndex];

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