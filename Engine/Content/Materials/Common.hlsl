static const float PI = 3.14159265359;

//struct VertexInputStaticMesh
//{
//    float3 Position : POSITION;
//    float3 Color : COLOR;
//    float3 Normal : NORMAL;
//    float3 Tangent : TANGENT;
//    float3 Bitangent : BINORMAL;
//    float2 UV1 : TEXCOORD0;
//    float2 UV2 : TEXCOORD1;
//    float2 UV3 : TEXCOORD2;
//    float2 UV4 : TEXCOORD3;
//};
//
//struct VertexInputPosColor
//{
//    float3 Position : POSITION;
//    float4 Color : COLOR;
//};
//
//struct GBuffer
//{
//    float4 BaseColor;
//    float4 WorldNormal;
//    float4 Mask;
//};
//
//struct BasePassPixelShaderOutput
//{
//    float4 ColorDeferred : SV_TARGET0;
//    float4 BaseColor : SV_TARGET1;
//    float4 WorldNormal : SV_TARGET2;
//    float4 Masks : SV_TARGET3;
//};
//
//struct PixelShaderOutput
//{
//#if MAIN_PASS
//    float4 ColorDeferred : SV_TARGET0;
//    float4 BaseColor : SV_TARGET1;
//    float4 WorldNormal : SV_TARGET2;
//    float4 Masks : SV_TARGET3;
//#elif HitProxyPass
//    uint4 Guid;
//#elif EDITOR_PRIMITIVE_PASS
//    float4 Color;
//#endif
//};
//
//struct ViewBuffer
//{
//    matrix LocalToProjection;
//    matrix LocalToWorld;
//    uint4 Guid;
//    matrix LocalToView;
//};
//
//float2 VSPosToScreenUV(float4 VSPos)
//{
//    float2 UV = VSPos.xy / VSPos.w;
//    UV = UV / 2 + 0.5f;
//    UV.y = 1 - UV.y;
//    
//    return UV;
//}

#define SHADING_MODEL_UNLIT 0
#define SHADING_MODEL_LIT 1
#define SHADING_MODEL_FOLIAGE 2

float Uint8ToFloat(uint Value)
{
    return Value / 255.0f;
}

uint FloatToUint8(float Value)
{
    return (uint) (Value * 255);
}

struct VertexInputPositionOnly
{
    float3 Position : POSITION;
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

struct BasePassPixelShaderOutput
{
#if MAIN_PASS
    float4 ColorDeferred : SV_TARGET0;
    float4 BaseColor : SV_TARGET1;
    float2 WorldNormal : SV_TARGET2;
    float4 Masks : SV_TARGET3;
    float4 MasksB : SV_TARGET4;
    float2 Velocity : SV_TARGET5;
#elif HITPROXY_PASS
    uint4 Guid;
#elif EDITOR_PRIMITIVE_PASS
    float4 Color;
#elif SHADOW_PASS
    
#elif PRE_PASS
    
#endif
};

float InterleavedGradientNoise(float2 uv, float FrameId)
{
    uv += FrameId * (float2(47, 17) * 0.695f);

    const float3 magic = float3(0.06711056f, 0.00583715f, 52.9829189f);
    return frac(magic.z * frac(dot(uv, magic.xy)));
}

float2 ViewportUVToScreenPos(float2 ViewportUV)
{
    return float2(2 * ViewportUV.x - 1, 1 - 2 * ViewportUV.y);
}

float2 ScreenPosToViewportUV(float2 ScreenPos)
{
    return float2(0.5 + 0.5 * ScreenPos.x, 0.5 - 0.5 * ScreenPos.y);
}

float2 SvPositionToViewportUV(float2 SvPosition, float2 InvSize)
{
    return (SvPosition * InvSize);
}

// Luma includes a scaling by 4.
float Luma4(float3 Color)
{
    return (Color.g * 2.0) + (Color.r + Color.b);
}

float2 EncodeNormal(float3 N)
{
    N.xy /= dot(1, abs(N));
    if (N.z <= 0)
    {
        N.xy = (1 - abs(N.yx)) * select(N.xy >= 0, float2(1, 1), float2(-1, -1));
    }
    return N.xy;
}

float3 DecodeNormal(float2 Oct)
{
    float3 N = float3(Oct, 1 - dot(1, abs(Oct)));
    if (N.z < 0)
    {
        N.xy = (1 - abs(N.yx)) * select(N.xy >= 0, float2(1, 1), float2(-1, -1));
    }
    return normalize(N);
}

//float3 DecodeNormal(float3 Normal)
//{
//    return Normal * 2 - 1;
//}
//
//float DistributionGGX(float3 N, float3 H, float roughness)
//{
//    float a = roughness * roughness;
//    float a2 = a * a;
//    float NdotH = max(dot(N, H), 0.0);
//    float NdotH2 = NdotH * NdotH;
//	
//    float num = a2;
//    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
//    denom = PI * denom * denom;
//	
//    return num / denom;
//}
//
//float GeometrySchlickGGX(float NdotV, float roughness)
//{
//    float r = (roughness + 1.0);
//    float k = (r * r) / 8.0;
//
//    float num = NdotV;
//    float denom = NdotV * (1.0 - k) + k;
//	
//    return num / denom;
//}
//float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
//{
//    float NdotV = max(dot(N, V), 0.0);
//    float NdotL = max(dot(N, L), 0.0);
//    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
//    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
//	
//    return ggx1 * ggx2;
//}
//
//float3 fresnelSchlick(float cosTheta, float3 F0)
//{
//    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
//}