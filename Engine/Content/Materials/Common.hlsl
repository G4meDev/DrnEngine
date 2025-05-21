
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

struct VertexInputPosColor
{
    float3 Position : POSITION;
    float4 Color : COLOR;
};

struct GBuffer
{
    float4 BaseColor;
    float4 WorldNormal;
    float4 Mask;
};

struct BasePassPixelShaderOutput
{
    float4 ColorDeferred : SV_TARGET0;
    float4 BaseColor : SV_TARGET1;
    float4 WorldNormal : SV_TARGET2;
    float4 Masks : SV_TARGET3;
};

// ------------------------------------------------------------

struct ViewBuffer
{
    matrix LocalToProjection;
    matrix LocalToWorld;
    uint4 Guid;
    matrix LocalToView;
};