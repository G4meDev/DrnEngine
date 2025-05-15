
struct StaticMeshVertexInput
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

struct VertexPosColor
{
    float3 Position : POSITION;
    float4 Color : COLOR;
};

// ------------------------------------------------------------

struct ViewBuffer
{
    matrix LocalToProjection;
    matrix LocalToWorld;
    uint4 Guid;
};