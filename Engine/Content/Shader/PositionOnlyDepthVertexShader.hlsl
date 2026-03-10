#include "Common.hlsl"

struct Resources
{
    uint ViewIndex;
    uint PrimitiveIndex;
    uint StaticSamplerBufferIndex;
    uint TextureBufferIndex;
    uint ScalarBufferIndex;
    uint VectorBufferIndex;
    uint ShadowDepthBuffer;
};

ConstantBuffer<Resources> BindlessResources : register(b0);

struct Primitive
{
    matrix LocalToWorld;
    matrix LocalToProjection;
    uint4 Guid;
    matrix PrevLocalToWorld;
    matrix PrevLocalToProjection;
};

struct VertexShaderOutput
{
    float4 Position : SV_Position;
};

VertexShaderOutput Main_VS(VertexInputPositionOnly IN)
{
    VertexShaderOutput OUT;
    
    ConstantBuffer<ViewBuffer> View = ResourceDescriptorHeap[BindlessResources.ViewIndex];
    ConstantBuffer<Primitive> P = ResourceDescriptorHeap[BindlessResources.PrimitiveIndex];
    
    matrix LocalToWorld;
    matrix LocalToProjection;
    
#if STATICMESH
    LocalToWorld = P.LocalToWorld;
    LocalToProjection = P.LocalToProjection;
#elif INSTANCED
    LocalToWorld = matrix((float3) IN.LocalToWorld1, (float3) IN.LocalToWorld2, (float3) IN.LocalToWorld3, IN.OriginRandom.xyz, float4(0, 0, 0, 1));
    LocalToProjection = mul(View.WorldToProjection, LocalToWorld);
#endif
    
    OUT.Position = mul(LocalToProjection, float4(IN.Position, 1.0f));
    
    return OUT;
}