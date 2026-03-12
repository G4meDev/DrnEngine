static const float PI = 3.14159265359;

#define FLT_MAX 3.402823466e+38
#define FLT_MIN 1.175494351e-38

#define VECTOR( name , displayname)         \
    float4 name;                            \

#define SCALAR( name , displayname)         \
    float name;                             \

#define TEX2D( name , displayname)          \
    uint name##_Texture;                    \
    uint name##_Sampler;                    \

#define TEXCUBE( name , displayname)        \
    uint name##_Texture;                    \
    uint name##_Sampler;                    \

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

struct StandardResources
{
    uint ViewIndex;
    uint PrimitiveIndex;
    uint StaticSamplerBufferIndex;
    uint ParametersBufferIndex;
    uint unused_1;
    uint unused_2;
    uint ShadowDepthBuffer;
    uint DecalBaseColor;
    uint DecalNormal;
    uint DecalMasks;
};

struct DecalResources
{
    uint ViewIndex;
#if DECAL
    uint DecalBufferIndex;
#elif STATICMESH
    uint MeshDecalBufferIndex;
#endif
    uint StaticSamplerBufferIndex;
    uint ParametersBufferIndex;
    uint unused_1;
    uint unused_2;
    uint DepthTexture;
};

struct DecalBuffer
{
    matrix LocalToProjection;
    matrix ProjectionToLocal;
};

struct MeshDecalBuffer
{
    matrix LocalToWorld;
    matrix LocalToProjection;
};

struct ViewBuffer
{
    matrix WorldToView;
    matrix ViewToProjection;
    matrix WorldToProjection;
    matrix ProjectionToView;
    matrix ProjectionToWorld;
    matrix LocalToCameraView;

    uint2 RenderSize;
    float2 InvSize;

    float3 CameraPos;
    float InvTanHalfFov;
		
    float3 CameraDir;
    float Pad_4;

    float4 InvDeviceZToWorldZTransform;
    matrix ViewToWorld;
    matrix ScreenToTranslatedWorld;
    
    uint FrameIndex;
    uint FrameIndexMod8;
    float2 JitterOffset;
    
    float2 PrevJitterOffset;
    float2 Pad_1;
    
    matrix ClipToPreviousClip;
};

struct PrimitiveBuffer
{
    matrix LocalToWorld;
    matrix LocalToProjection;
    uint4 Guid;
    matrix PrevLocalToWorld;
    matrix PrevLocalToProjection;
};

struct StaticSamplers
{
    uint LinearSamplerIndex;
    uint PointSamplerIndex;
    uint LinearCmpSamplerIndex;
    uint LinearClampIndex;
    uint PointClampIndex;
};

#if SHADOW_PASS_POINTLIGHT
struct ShadowDepth
{
    matrix WorldToProjectionMatrices[6];
};
#elif SHADOW_PASS_SPOTLIGHT
struct ShadowDepth
{
    matrix WorldToProjectionMatrix;
};
#endif

struct VertexInputPositionOnlyStaticMesh
{
    float3 Position : POSITION;
};

struct VertexInputPositionOnlyInstancedStaticMesh
{
    float3 Position : POSITION;
    
    float4 OriginRandom : ORIGIN_RANDOM;
    half4 LocalToWorld1 : MAT1;
    half4 LocalToWorld2 : MAT2;
    half4 LocalToWorld3 : MAT3;
};

#if INSTANCED
#define VertexInputPositionOnly VertexInputPositionOnlyInstancedStaticMesh
#elif STATICMESH
#define VertexInputPositionOnly VertexInputPositionOnlyStaticMesh
#endif

struct VertexInputStaticMesh
{
    float3 Position : POSITION;
    float3 Color : COLOR;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float2 UV1 : TEXCOORD0;
    float2 UV2 : TEXCOORD1;
    float2 UV3 : TEXCOORD2;
    float2 UV4 : TEXCOORD3;
};

struct VertexInputInstancedStaticMesh
{
    float3 Position : POSITION;
    float3 Color : COLOR;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float2 UV1 : TEXCOORD0;
    float2 UV2 : TEXCOORD1;
    float2 UV3 : TEXCOORD2;
    float2 UV4 : TEXCOORD3;
    
    float4 OriginRandom : ORIGIN_RANDOM;
    half4 LocalToWorld1 : MAT1;
    half4 LocalToWorld2 : MAT2;
    half4 LocalToWorld3 : MAT3;
    
    float4 PerInstanceCustom1 : CUSTOM1;
    float4 PerInstanceCustom2 : CUSTOM2;
};

#if INSTANCED
#define VertexInput VertexInputInstancedStaticMesh
#elif STATICMESH
#define VertexInput VertexInputStaticMesh
#endif

struct BasePassPixelShaderOutput
{
#if MAIN_PASS
    float4 ColorDeferred : SV_TARGET0;
    float4 BaseColor : SV_TARGET1;
    float2 WorldNormal : SV_TARGET2;
    float4 Masks : SV_TARGET3;
    float4 MasksB : SV_TARGET4;
#elif HITPROXY_PASS
    uint4 Guid;
#elif EDITOR_PRIMITIVE_PASS
    float4 Color;
#elif SHADOW_PASS
    
#elif PRE_PASS
    
#endif
};

matrix GetLocalToWorld(VertexInputInstancedStaticMesh IN)
{
    return matrix
        ( float4(IN.LocalToWorld1.x, IN.LocalToWorld2.x, IN.LocalToWorld3.x, IN.OriginRandom.x)
        , float4(IN.LocalToWorld1.y, IN.LocalToWorld2.y, IN.LocalToWorld3.y, IN.OriginRandom.y)
        , float4(IN.LocalToWorld1.z, IN.LocalToWorld2.z, IN.LocalToWorld3.z, IN.OriginRandom.z)
        , float4(0, 0, 0, 1));
}

matrix GetLocalToWorld(VertexInputPositionOnlyInstancedStaticMesh IN)
{
    return matrix
        ( float4(IN.LocalToWorld1.x, IN.LocalToWorld2.x, IN.LocalToWorld3.x, IN.OriginRandom.x)
        , float4(IN.LocalToWorld1.y, IN.LocalToWorld2.y, IN.LocalToWorld3.y, IN.OriginRandom.y)
        , float4(IN.LocalToWorld1.z, IN.LocalToWorld2.z, IN.LocalToWorld3.z, IN.OriginRandom.z)
        , float4(0, 0, 0, 1));
}

uint ReverseBits32( uint bits )
{
#if SM5_PROFILE || COMPILER_METAL
	return reversebits( bits );
#else
	bits = ( bits << 16) | ( bits >> 16);
	bits = ( (bits & 0x00ff00ff) << 8 ) | ( (bits & 0xff00ff00) >> 8 );
	bits = ( (bits & 0x0f0f0f0f) << 4 ) | ( (bits & 0xf0f0f0f0) >> 4 );
	bits = ( (bits & 0x33333333) << 2 ) | ( (bits & 0xcccccccc) >> 2 );
	bits = ( (bits & 0x55555555) << 1 ) | ( (bits & 0xaaaaaaaa) >> 1 );
	return bits;
#endif
}

float ConvertFromDeviceZ(float DeviceZ, float4 InvDeviceZToWorldZTransform)
{
    return DeviceZ * InvDeviceZToWorldZTransform[0] + InvDeviceZToWorldZTransform[1] + 1.0f / (DeviceZ * InvDeviceZToWorldZTransform[2] - InvDeviceZToWorldZTransform[3]);
}

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

float3 ReconstructNormal(float2 xz)
{
    return float3(xz.x, sqrt(1 - dot(xz, xz)), xz.y);
}

float3 ReconstructTextureNormal(float2 xy, bool bInvertY = true)
{
    if(bInvertY)
    {
        xy.y = 1 - xy.y;
    }
    float2 normalxy = xy * 2 - 1;
    return float3(normalxy.x, sqrt(1 - dot(normalxy, normalxy)), normalxy.y);
}

float3x3 GetTBN(float3 WorldNormal, float3 WorldTangent)
{
    float3 WorldBinormal = normalize(cross(WorldTangent, WorldNormal));
    return float3x3(WorldTangent, WorldNormal, WorldBinormal);
}

float Pow4( float x )
{
	float xx = x*x;
	return xx * xx;
}

float Square(float x) { return x * x; }

float3x3 GetTangentBasis(float3 TangentY)
{
    const float Sign = TangentY.y >= 0 ? 1 : -1;
    const float a = -rcp(Sign + TangentY.y);
    const float b = TangentY.x * TangentY.z * a;

    float3 TangentX = { 1 + Sign * a * Square(TangentY.x), -Sign * TangentY.x, Sign * b };
    float3 TangentZ = { b, -TangentY.z, Sign + a * Square(TangentY.z) };
    
    return float3x3(TangentX, TangentY, TangentZ);
}

float ComputeReflectionCaptureRoughnessFromMip(float Mip, half CubemapMaxMip)
{
    float LevelFrom1x1 = CubemapMaxMip - 1.0f - Mip;
    return exp2((1.0f - LevelFrom1x1) / 1.2f);
}

half ComputeReflectionCaptureMipFromRoughness(half Roughness, half CubemapMaxMip)
{
    half LevelFrom1x1 = 1.0f - 1.2 * log2(max(Roughness, 0.001));
    return CubemapMaxMip - 1 - LevelFrom1x1;
}

float2 Hammersley( uint Index, uint NumSamples, uint2 Random )
{
	float E1 = frac( (float)Index / NumSamples + float( Random.x & 0xffff ) / (1<<16) );
	float E2 = float( ReverseBits32(Index) ^ Random.y ) * 2.3283064365386963e-10;
	return float2( E1, E2 );
}

float4 CosineSampleHemisphere( float2 E )
{
	float Phi = 2 * PI * E.x;
	float CosTheta = sqrt(E.y);
	float SinTheta = sqrt(1 - CosTheta * CosTheta);

	float3 H;
	H.x = SinTheta * cos(Phi);
	H.z = SinTheta * sin(Phi);
	H.y = CosTheta;

	float PDF = CosTheta * (1.0 / PI);

	return float4(H, PDF);
}

float4 ImportanceSampleGGX( float2 E, float a2 )
{
	float Phi = 2 * PI * E.x;
	float CosTheta = sqrt( (1 - E.y) / ( 1 + (a2 - 1) * E.y ) );
	float SinTheta = sqrt( 1 - CosTheta * CosTheta );

	float3 H;
	H.x = SinTheta * cos( Phi );
	H.z = SinTheta * sin( Phi );
	H.y = CosTheta;
	
	float d = ( CosTheta * a2 - CosTheta ) * CosTheta + 1;
	float D = a2 / ( PI*d*d );
	float PDF = D * CosTheta;

	return float4( H, PDF );
}

float D_GGX(float a2, float NoH)
{
    float d = (NoH * a2 - NoH) * NoH + 1;
    return a2 / (PI * d * d);
}

float CameraDepthFade(float3 WorldPosition, float3 CameraPosition, float3 CameraDirection, float DepthOffset, float DepthLength)
{
    float3 CameraToWorldPosition = WorldPosition - CameraPosition;
    float Depth = length(CameraToWorldPosition) * (dot(normalize(CameraToWorldPosition), CameraDirection));
    
    return saturate((Depth - DepthOffset) / DepthLength);
}

float CameraDepthFade(float PixelDepth, float DepthOffset, float DepthLength)
{
    return saturate((PixelDepth - DepthOffset) / DepthLength);
}

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