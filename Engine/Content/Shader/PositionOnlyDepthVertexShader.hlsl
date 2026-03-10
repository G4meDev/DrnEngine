//#define INSTANCED 1
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
    LocalToWorld = matrix
        ( float4(IN.LocalToWorld1.x, IN.LocalToWorld2.x, IN.LocalToWorld3.x, IN.OriginRandom.x)
        , float4(IN.LocalToWorld1.y, IN.LocalToWorld2.y, IN.LocalToWorld3.y, IN.OriginRandom.y)
        , float4(IN.LocalToWorld1.z, IN.LocalToWorld2.z, IN.LocalToWorld3.z, IN.OriginRandom.z)
        , float4(0 , 0, 0, 1));
    LocalToProjection = mul(View.WorldToProjection, LocalToWorld);
#endif
    
    OUT.Position = mul(LocalToProjection, float4(IN.Position, 1.0f));
    
    return OUT;
}