//#include "../../../Engine/Content/Materials/Common.hlsl"

// SUPPORT_EDITOR_PRIMITIVE_PASS

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

struct ViewBuffer
{
    matrix WorldToView;
    matrix ViewToProjection;
    matrix WorldToProjection;
    matrix ProjectionToView;
    matrix ProjectionToWorld;
    matrix LocalToCameraView;

    uint2 RenderSize;
};

struct Primitive
{
    matrix LocalToWorld;
    matrix LocalToProjection;
    uint4 Guid;
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

struct PixelShaderOutput
{
#if MAIN_PASS
    float4 ColorDeferred : SV_TARGET0;
    float4 BaseColor : SV_TARGET1;
    float4 WorldNormal : SV_TARGET2;
    float4 Masks : SV_TARGET3;
#elif HitProxyPass
    uint4 Guid;
#elif EDITOR_PRIMITIVE_PASS
    float4 Color;
#endif
};

struct VertexShaderOutput
{
    float3 WorldPos : WORLD_POS;
    float4 Position : SV_Position;
};

VertexShaderOutput Main_VS(VertexInputStaticMesh IN)
{
    VertexShaderOutput OUT;

    ConstantBuffer<Primitive> P = ResourceDescriptorHeap[BindlessResources.PrimitiveIndex];
    
    OUT.Position = mul(P.LocalToProjection, float4(IN.Position, 1.0f));
    OUT.WorldPos = mul(P.LocalToWorld, float4(IN.Position, 1.0f)).xyz;

    return OUT;
}

// -------------------------------------------------------------------------------------

struct PixelShaderInput
{
    float3 WorldPos : WORLD_POS;
};

PixelShaderOutput Main_PS(PixelShaderInput IN) : SV_Target
{
    PixelShaderOutput OUT;

    float2 Pos_xz = IN.WorldPos.xz;

    float2 Frac1 = frac(Pos_xz);
    float2 FracEdge1 = abs(Frac1 - 0.5f) - 0.47f;
    float Alpha1 = max(FracEdge1.x, FracEdge1.y) * 4;

    float2 Frac2 = frac(Pos_xz / 10);
    float2 FracEdge2 = abs(Frac2 - 0.5f) - 0.49f;
    float Alpha2 = max(FracEdge2.x, FracEdge2.y) * 20;

    float Alpha = max(Alpha1, Alpha2);
    clip(Alpha < 0.0f ? -1 : 1);
    
    OUT.Color = float4(1, 1, 1, Alpha.x);

    
    return OUT;
}