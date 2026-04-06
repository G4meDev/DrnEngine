static const float PI = 3.14159265359;

#define FLT_MAX 3.402823466e+38
#define FLT_MIN 1.175494351e-38

// max 16 bit/float4s in cnstant buffer
#define D3D12_REQ_CONSTANT_BUFFER_ELEMENT_COUNT 4096

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

struct GBufferTextures
{
    uint DepthIndex;
    uint DeferredColorIndex;
    uint BaseColorIndex;
    uint NormalIndex;
    uint MasksAIndex;
    uint MasksBIndex;
    uint VelocityIndex;
};

struct GBufferData
{
    float3 BaseColor;
    float3 WorldNormal;
    float Matallic;
    float Roughness;
    float AmbientOcclusion;
    float3 TransmittanceColor;
    uint ShadingModel;
};


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

struct TranslucentResources
{
    uint ViewIndex;
    uint PrimitiveIndex;
    uint StaticSamplerBufferIndex;
    uint ParametersBufferIndex;
    uint GbufferTextureIndex;
    uint LightGridIndex;
    
};

struct DecalResources
{
    uint ViewIndex;
#if DECAL
    uint DecalBufferIndex;
#elif STATICMESH
    uint PrimitiveIndex;
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
    float AspectRatio;

    float4 InvDeviceZToWorldZTransform;
    matrix ViewToWorld;
    matrix ScreenToTranslatedWorld;
    
    uint FrameIndex;
    uint FrameIndexMod8;
    float2 JitterOffset;
    
    float2 PrevJitterOffset;
    float2 Pad_1;
    
    matrix ClipToPreviousClip;
    matrix PrevWorldToProjection;
};

struct PrimitiveBuffer
{
    matrix LocalToWorld;
    matrix PrevLocalToWorld;
    uint4 Guid;
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

#define LIGHT_GRID_LOCAL_LIGHT_DATA_STRIDE 5
#define LIGHT_GRID_REFLECTION_CAPTURE_DATA_STRIDE 3
#define LIGHT_GRID_MAX_LOCAL_LIGHTS D3D12_REQ_CONSTANT_BUFFER_ELEMENT_COUNT / LIGHT_GRID_LOCAL_LIGHT_DATA_STRIDE
#define LIGHT_GRID_MAX_REFLECTION_CAPTURES D3D12_REQ_CONSTANT_BUFFER_ELEMENT_COUNT / LIGHT_GRID_LOCAL_LIGHT_DATA_STRIDE

#define LIGHT_GRID_LIGHT_TYPE_DIRECTIONAL	0
#define LIGHT_GRID_LIGHT_TYPE_POINT			1
#define LIGHT_GRID_LIGHT_TYPE_SPOT			2
#define LIGHT_GRID_LIGHT_TYPE_MAX			3

// num lights, startoffset
#define LIGHT_GRID_LINK_STRIDE 2

struct LightGridDirectionalLightData
{
    uint HasDirectionalLight;
    float3 LightColor;
    float3 LightDirection;
};

struct LightGridLocalLightData
{
    float4 LightPositionAndInvRadius;
	float4 LightColorAndFalloffExponent;
    float4 LightDirectionAndLightType;
	float4 SpotAnglesAndSourceRadiusPacked;
    float4 LightTangentAndSoftSourceRadius;
};

struct LightGridReflectionCaptureData
{
    float4 PositionAndRadius;
    float4 CaptureOffsetBrightness;
    uint CubemapIndex;
};

struct LightGridData
{
    float3 DirectionalLightColor;
    uint HasDirectionalLight;
    float3 DirectionalLightDirection;
    uint LocalLightBufferIndex;
    
    int3 CulledGridSize;
    uint NumCulledLights;
    
    uint NumGridCells;
    uint MaxCulledLightsPerCell;
    uint LightGridPixelSizeShift;
    uint ViewSpacePositionAndRadiusIndex;

    float3 LightGridZParams;
    uint ViewSpaceDirAndPreprocAngleIndex;
    
    uint RWNextCulledLightLinkIndex;
    uint RWCulledLightLinkIndex;
    uint RWStartGridOffsetIndex;
    uint RWNextCulledLightDataIndex;
    
    uint CulledLightLinkIndex;
    uint StartGridOffsetIndex;
    uint RWLightGridNumOffsetIndex;
    uint RWLightGridLinkListIndex;
    
    uint LightGridNumOffsetIndex;
    uint LightGridLinkListIndex;
// ------------------------------------------
    uint HasSkyLight;
    uint SkyLightConvolutionIndex;
    
    float3 SkyLightColor;
    uint SkyLightIrradianceIndex;
    
    uint SkyLightMipCount;
    uint PreintegeratedGFImageIndex;
    uint NumReflectionCaptures;
    uint ReflectionCaptureBufferIndex;
};

struct LightGridPackedLocalLightData
{
    float4 LocalLightBuffer[LIGHT_GRID_MAX_LOCAL_LIGHTS * LIGHT_GRID_LOCAL_LIGHT_DATA_STRIDE];
};

struct LightGridViewSpacePositionAndRadius
{
    float4 PackedData[LIGHT_GRID_MAX_LOCAL_LIGHTS];
};

struct LightGridViewSpaceDirAndPreprocAngle
{
    float4 PackedData[LIGHT_GRID_MAX_LOCAL_LIGHTS];
};

struct LightGridPackedReflectionCaptureData
{
    float4 LocalLightBuffer[LIGHT_GRID_MAX_REFLECTION_CAPTURES * LIGHT_GRID_REFLECTION_CAPTURE_DATA_STRIDE];
};

LightGridDirectionalLightData GetDirectionalLightData(LightGridData LightGrid)
{
    LightGridDirectionalLightData Result;
    
    Result.HasDirectionalLight = LightGrid.HasDirectionalLight;
    Result.LightColor = LightGrid.DirectionalLightColor;
    Result.LightDirection = LightGrid.DirectionalLightDirection;
    
    return Result;
}

LightGridLocalLightData GetLocalLightData(LightGridPackedLocalLightData PackedLocalLights, uint LightIndex)
{
    LightGridLocalLightData Result;
    
    uint LocalLightBaseIndex = LightIndex * LIGHT_GRID_LOCAL_LIGHT_DATA_STRIDE;
    Result.LightPositionAndInvRadius = PackedLocalLights.LocalLightBuffer[LocalLightBaseIndex + 0];
    Result.LightColorAndFalloffExponent = PackedLocalLights.LocalLightBuffer[LocalLightBaseIndex + 1];
    Result.LightDirectionAndLightType = PackedLocalLights.LocalLightBuffer[LocalLightBaseIndex + 2];
    Result.SpotAnglesAndSourceRadiusPacked = PackedLocalLights.LocalLightBuffer[LocalLightBaseIndex + 3];
    Result.LightTangentAndSoftSourceRadius = PackedLocalLights.LocalLightBuffer[LocalLightBaseIndex + 4];
    
    return Result;
}

LightGridReflectionCaptureData GetReflectionCaptureData(LightGridPackedReflectionCaptureData PackedReflectionCaptures, uint ReflectionCaptureIndex)
{
    LightGridReflectionCaptureData Result;
    
    uint ReflectionCaptureBaseIndex = ReflectionCaptureIndex * LIGHT_GRID_REFLECTION_CAPTURE_DATA_STRIDE;
    Result.PositionAndRadius = PackedReflectionCaptures.LocalLightBuffer[ReflectionCaptureBaseIndex + 0];
    Result.CaptureOffsetBrightness = PackedReflectionCaptures.LocalLightBuffer[ReflectionCaptureBaseIndex + 1];
    Result.CubemapIndex = asuint(PackedReflectionCaptures.LocalLightBuffer[ReflectionCaptureBaseIndex + 2].x);
    
    return Result;
}

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

float max3(float A, float B, float C)
{
    return max(max(A, B), C);
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

float ConvertToDeviceZ(float SceneDepth, ViewBuffer View)
{
    //[branch]
    //if (View.ViewToClip[3][3] < 1.0f)
    //{
		// Perspective
        //return 1.0f / ((SceneDepth + View.InvDeviceZToWorldZTransform[3]) * View.InvDeviceZToWorldZTransform[2]);
        return ((1 / SceneDepth) + View.InvDeviceZToWorldZTransform[3]) / View.InvDeviceZToWorldZTransform[2];
    //}
    //else
    //{
	//	// Ortho
    //    return SceneDepth * View.ViewToClip[2][2] + View.ViewToClip[3][2];
    //}
}

float InterleavedGradientNoise(float2 uv, float FrameId)
{
    uv += FrameId * (float2(47, 17) * 0.695f);

    const float3 magic = float3(0.06711056f, 0.00583715f, 52.9829189f);
    return frac(magic.z * frac(dot(uv, magic.xy)));
}

#define BBS_PRIME24 4093
float RandBBSfloat(float seed)
{
    float s = frac(seed / BBS_PRIME24);
    s = frac(s * s * BBS_PRIME24);
    s = frac(s * s * BBS_PRIME24);
    return s;
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

float2 VSPosToScreenUV(float4 VSPos)
{
    float2 UV = VSPos.xy / VSPos.w;
    UV = UV / 2 + 0.5f;
    UV.y = 1 - UV.y;
    
    return UV;
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

float CheapContrast(float Value, float Contrast)
{
    return saturate(lerp(-Contrast, Contrast, Value));
}

float4 WorldAlignedTexture(float3 WorldPosition, float3 TextureSize, Texture2D Texture, SamplerState Sampler, float3 WorldNormal, float TransitionContrast)
{
    float3 ScaledPosition = WorldPosition / TextureSize;
    float4 XProjected = Texture.Sample(Sampler, ScaledPosition.zy);
    float4 YProjected = Texture.Sample(Sampler, ScaledPosition.xz);
    float4 ZProjected = Texture.Sample(Sampler, ScaledPosition.xy);
    
    float ZTransition = CheapContrast(abs(WorldNormal.z), TransitionContrast);
    float YTransition = CheapContrast(abs(WorldNormal.y), TransitionContrast);
    
    float4 Result = lerp(XProjected, ZProjected, ZTransition);
    Result = lerp(Result, YProjected, YTransition);
    
    return Result;
}

float3 Calculate3DVelocity(float4 PackedVelocityA, float4 PackedVelocityC, float2 TaaJitter, float2 PrevTaaJitter)
{
    float2 ScreenPos = PackedVelocityA.xy / PackedVelocityA.w - TaaJitter;
    float2 PrevScreenPos = PackedVelocityC.xy / PackedVelocityC.w - PrevTaaJitter;

    float DeviceZ = PackedVelocityA.z / PackedVelocityA.w;
    float PrevDeviceZ = PackedVelocityC.z / PackedVelocityC.w;

    float3 Velocity = float3(ScreenPos - PrevScreenPos, DeviceZ - PrevDeviceZ);
    return Velocity;
}

float2 EncodeVelocityToTexture(float2 V)
{
    float2 EncodedV;
    EncodedV.xy = V.xy * (0.499f * 0.5f) + 32767.0f / 65535.0f;
    return EncodedV;
}

float2 DecodeVelocityFromTexture(float2 EncodedV)
{
    const float InvDiv = 1.0f / (0.499f * 0.5f);

    float2 V;
    V.xy = EncodedV.xy * InvDiv - 32767.0f / 65535.0f * InvDiv;
    return V;
}

float Fresnel_Function(float3 Normal, float3 CameraVector, float Power)
{
    float NdotC = saturate(dot(Normal, CameraVector));
    float Result = pow(1 - NdotC, Power);
    
    return Result;
}

float2 ComputeBufferUVDistortion(ViewBuffer View, half3 Normal, float IOR)
{
    half3 ViewNormal = normalize(mul((float3x3) View.WorldToView, Normal));
    float AirIOR = 1.0f;
    float2 ViewportUVDistortion = ViewNormal.xy * (IOR - AirIOR);
    
    float2 BufferUVDistortion = ViewportUVDistortion;
#if DISTORTION_PASS
    clip(dot(BufferUVDistortion, BufferUVDistortion) - .00001);
#endif

    float4 DistortionParameters = float4(View.InvTanHalfFov, View.AspectRatio, View.RenderSize);
    
    float InvTanHalfFov = DistortionParameters.x;
    float Ratio = DistortionParameters.y;
    
    float2 FovFix = float2(InvTanHalfFov, Ratio * InvTanHalfFov);

    const float OffsetFudgeFactor = 0.00023;
    BufferUVDistortion *= DistortionParameters.zw * float2(OffsetFudgeFactor, -OffsetFudgeFactor) * FovFix;

    return BufferUVDistortion;
}

void PostProcessUVDistortion(float4 ScreenPosition, float DistortSceneDepth, float RefractionBias, inout float2 BufferUVDistortion)
{
    float Bias = -RefractionBias;
    float Range = clamp(abs(Bias * 0.5f), 0, 50);
    float Z = DistortSceneDepth;
    float ZCompare = ScreenPosition.w;
    float InvWidth = 1.0f / max(1.0f, Range);
    BufferUVDistortion *= saturate((Z - ZCompare) * InvWidth + Bias);

    static const half DistortionScaleBias = 4.0f;
    BufferUVDistortion *= DistortionScaleBias;
}

// --------------------------------------------------------------------------------------------------

float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
	
    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}
float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

float3 fresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// --------------------------------------------------------------------------------------------------

struct PointLightData
{
    float4 WorldPosAndScale;
    float3 Color;
    float InvRadius;
    uint ShadowDataIndex; // 0 if not casting shadow
};

struct PointLightShadowData
{
    matrix WorldToProjectionMatrices[6];
    uint ShadowMapTextureIndex;
    float DepthBias;
    float InvShadowmapResolution;
};

struct SpotLightData
{
    matrix LocalToWorld;
    float3 WorldPosition;
    float Attenution;
    float3 Direction;
    float InvRadius;
    float3 Color;
    float OuterRadius;
    float InnerRadius;
    float CosOuterCone;
    float InvCosConeDifference;
    
    uint ShadowDataIndex; // 0 if not casting shadow
};

struct SpotLightShadowData
{
    matrix WorldToProjectionMatrix;
    uint ShadowMapTextureIndex;
    float DepthBias;
    float InvShadowmapResolution;
};

struct DirectionalLightData
{
    float3 Direction;
    uint ShadowDataIndex; // 0 if not casting shadow
    float3 Color;
};

struct DirectionalLightShadowData
{
    float DepthBias;
    float InvShadowmapResolution;
    uint CascadeCount;
    uint ShadowMapTextureIndex;
    
    matrix CsWorldToProjectionMatrices[8];
    float4 CsSplitDistances[2];
};

static const float2 DiscSamples5[] =
{
    float2(0.000000, 2.500000),
	float2(2.377641, 0.772542),
	float2(1.469463, -2.022543),
	float2(-1.469463, -2.022542),
	float2(-2.377641, 0.772543),
};

static const float2 DiscSamples12[] =
{
    float2(0.000000, 2.500000),
	float2(1.767767, 1.767767),
	float2(2.500000, -0.000000),
	float2(1.767767, -1.767767),
	float2(-0.000000, -2.500000),
	float2(-1.767767, -1.767767),
	float2(-2.500000, 0.000000),
	float2(-1.767766, 1.767768),
	float2(-1.006119, -0.396207),
	float2(1.000015, 0.427335),
	float2(0.416807, -1.006577),
	float2(-0.408872, 1.024430),
};

static const float2 DiscSamples29[] =
{
    float2(0.000000, 2.500000),
	float2(1.016842, 2.283864),
	float2(1.857862, 1.672826),
	float2(2.377641, 0.772542),
	float2(2.486305, -0.261321),
	float2(2.165063, -1.250000),
	float2(1.469463, -2.022543),
	float2(0.519779, -2.445369),
	float2(-0.519779, -2.445369),
	float2(-1.469463, -2.022542),
	float2(-2.165064, -1.250000),
	float2(-2.486305, -0.261321),
	float2(-2.377641, 0.772543),
	float2(-1.857862, 1.672827),
	float2(-1.016841, 2.283864),
	float2(0.091021, -0.642186),
	float2(0.698035, 0.100940),
	float2(0.959731, -1.169393),
	float2(-1.053880, 1.180380),
	float2(-1.479156, -0.606937),
	float2(-0.839488, -1.320002),
	float2(1.438566, 0.705359),
	float2(0.067064, -1.605197),
	float2(0.728706, 1.344722),
	float2(1.521424, -0.380184),
	float2(-0.199515, 1.590091),
	float2(-1.524323, 0.364010),
	float2(-0.692694, -0.086749),
	float2(-0.082476, 0.654088),
};

float3 CalculateLighting(float3 L, float3 N, float3 NormalizedCameraVector, float3 LightColor, float3 BaseColor, float Metallic, float Roughness)
{
    float3 H = normalize(NormalizedCameraVector + L);
    float3 NoL = saturate(dot(N, L));
    
    float3 F0 = float3(0.04, 0.04, 0.04);
    F0 = lerp(F0, BaseColor, Metallic);
    
    float NDF = DistributionGGX(N, H, Roughness);
    float G = GeometrySmith(N, NormalizedCameraVector, L, Roughness);
    float3 F = fresnelSchlick(max(dot(H, NormalizedCameraVector), 0.0), F0);
    
    float3 kS = F;
    float3 kD = float3(1, 1, 1) - kS;
    kD *= 1.0 - Metallic;
    
    float3 Specular = NDF * G * F / (max(dot(N, NormalizedCameraVector), 0) * NoL + 0.0001);
    
    return (kD * BaseColor / PI + Specular) * NoL * LightColor;
}

float3 CalculatePointLightRadiance(float3 WorldPosition, float3 LightPosition, float3 LightColor, float3 CameraVector, GBufferData Gbuffer)
{
    float3 ToLight = LightPosition - WorldPosition;
    float DistanceSquare = dot(ToLight, ToLight);
    float3 L = ToLight * rsqrt(DistanceSquare);
    
    [branch]
    if (Gbuffer.ShadingModel == SHADING_MODEL_FOLIAGE && dot(Gbuffer.WorldNormal, -L) > 0.0f)
    {
        return CalculateLighting(-L, Gbuffer.WorldNormal, normalize(CameraVector), LightColor, Gbuffer.TransmittanceColor, Gbuffer.Matallic, Gbuffer.Roughness);
    }
    else
    {
        return CalculateLighting(L, Gbuffer.WorldNormal, normalize(CameraVector), LightColor, Gbuffer.BaseColor, Gbuffer.Matallic, Gbuffer.Roughness);
    }
}

float3 CalculateDirectionalLightRadiance(float3 WorldPosition, float3 LightDirection, float3 LightColor, float3 CameraVector, GBufferData Gbuffer)
{
    [branch]
    if (Gbuffer.ShadingModel == SHADING_MODEL_FOLIAGE && dot(Gbuffer.WorldNormal, LightDirection) > 0.0f)
    {
        return CalculateLighting(LightDirection, Gbuffer.WorldNormal, normalize(CameraVector), LightColor, Gbuffer.TransmittanceColor, Gbuffer.Matallic, Gbuffer.Roughness);
    }
    else
    {
        return CalculateLighting(-LightDirection, Gbuffer.WorldNormal, normalize(CameraVector), LightColor, Gbuffer.BaseColor, Gbuffer.Matallic, Gbuffer.Roughness);
    }
}

float CalculatePointLightAttenuation(float3 WorldPosition, float3 LightPosition, float InvRadius)
{
    float Distance = distance(WorldPosition, LightPosition);
    float Distance2 = Distance * Distance;
    float DistanceAttenuation = 1 / (Distance2 + 1);

    float LightRadiusMask = Distance2 * InvRadius * InvRadius;

    LightRadiusMask *= LightRadiusMask;
    LightRadiusMask = 1 - LightRadiusMask;
    LightRadiusMask = clamp(LightRadiusMask, 0, 1);

    LightRadiusMask *= LightRadiusMask;

    return DistanceAttenuation * LightRadiusMask;
}

float CalculateSpotLightAttenuation(float3 WorldPosition, float3 LightPosition, float InvRadius, float3 Direction, float CosOuterCone, float InvCosConeDifference)
{
    float RadialAttenuation = CalculatePointLightAttenuation(WorldPosition, LightPosition, InvRadius);
    
    float3 L = normalize(LightPosition - WorldPosition);
    float SpotAttenuationMask = saturate((dot(L, -Direction) - CosOuterCone) * InvCosConeDifference);
    float ConeAngleFalloff = SpotAttenuationMask * SpotAttenuationMask;
    
    return RadialAttenuation * ConeAngleFalloff;
}

float CalculatePointLightShadow(float3 WorldPosition, float3 LightPosition, matrix WorldToProjectionMatrices[6],
    float DepthBias, float InvShadowmapResolution, TextureCube ShadowmapTexture, SamplerComparisonState Sampler)
{
    float3 ToLight = LightPosition - WorldPosition;
    float3 AbsLightVector = abs(ToLight);
    float MaxCoordinate = max(AbsLightVector.x, max(AbsLightVector.y, AbsLightVector.z));

    float3 NormalizedToLight = ToLight / length(ToLight);
    float3 SideVector = normalize(cross(NormalizedToLight, float3(0, 1, 0)));
    float3 UpVector = normalize(cross(SideVector, NormalizedToLight));
        
    SideVector *= InvShadowmapResolution;
    UpVector *= InvShadowmapResolution;
        
    int CubeFaceIndex = 0;
    if (MaxCoordinate == AbsLightVector.x)
    {
        CubeFaceIndex = AbsLightVector.x == ToLight.x ? 1 : 0;
    }
    else if (MaxCoordinate == AbsLightVector.y)
    {
        CubeFaceIndex = AbsLightVector.y == ToLight.y ? 3 : 2;
    }
    else
    {
        CubeFaceIndex = AbsLightVector.z == ToLight.z ? 5 : 4;
    }
        
    float4 ShadowPos = mul(WorldToProjectionMatrices[CubeFaceIndex], float4(WorldPosition, 1));

    float CompareDistance = ShadowPos.z / ShadowPos.w;
    float ShadowDepthBias = -DepthBias / ShadowPos.w;

    float Shadow = 0;

#define SHADOW_QUALITY 3
        
#if SHADOW_QUALITY == 0
        Shadow = 1;

#elif SHADOW_QUALITY == 1
        Shadow = ShadowmapTexture.SampleCmp(Sampler, -ToLight, CompareDistance + ShadowDepthBias);

#elif SHADOW_QUALITY == 2
        
        [unroll]
        for (int i = 0; i < 5; ++i)
        {
            float3 SamplePos = NormalizedToLight + SideVector * DiscSamples5[i].x + UpVector * DiscSamples5[i].y;
            Shadow += ShadowmapTexture.SampleCmp(Sampler, -SamplePos, CompareDistance + ShadowDepthBias * length(DiscSamples5[i]));
        }
        Shadow /= 5;
        
#elif SHADOW_QUALITY == 3
        
    [unroll]
    for (int i = 0; i < 12; ++i)
    {
        float3 SamplePos = NormalizedToLight + SideVector * DiscSamples12[i].x + UpVector * DiscSamples12[i].y;
        Shadow += ShadowmapTexture.SampleCmp(Sampler, -SamplePos, CompareDistance + ShadowDepthBias * length(DiscSamples12[i]));
    }
    Shadow /= 12;

#elif SHADOW_QUALITY == 4
        [unroll]
        for (int i = 0; i < 29; ++i)
        {
            float3 SamplePos = NormalizedToLight + SideVector * DiscSamples29[i].x + UpVector * DiscSamples29[i].y;
            Shadow += ShadowmapTexture.SampleCmp(Sampler, -SamplePos, CompareDistance + ShadowDepthBias * length(DiscSamples29[i]));
        }
        Shadow /= 29;

#endif
    
#undef SHADOW_QUALITY
    
    return Shadow;
}

float CalculateSpotLightShadow(float3 WorldPosition, float3 ToLight, matrix WorldToProjectionMatrix,
    float DepthBias, float InvShadowmapResolution, Texture2D ShadowmapTexture, SamplerComparisonState Sampler)
{
    float4 ShadowPos = mul(WorldToProjectionMatrix, float4(WorldPosition, 1));

    float CompareDistance = ShadowPos.z / ShadowPos.w;
    float ShadowDepthBias = -DepthBias / ShadowPos.w;
    float2 ShadowmapUV = ShadowPos.xy / ShadowPos.w;
    ShadowmapUV = ShadowmapUV * 0.5f + 0.5f;
    ShadowmapUV.y = 1 - ShadowmapUV.y;
        
    float2 SideVector = mul(WorldToProjectionMatrix, float4(ToLight, 0)).xy;
    SideVector = normalize(SideVector);
    float2 UpVector = normalize(float2(SideVector.y, -SideVector.x));
    
    //float2 SideVector = float2(1, 0);
    //float2 UpVector = float2(0, 1);
    
    SideVector *= InvShadowmapResolution;
    UpVector *= InvShadowmapResolution;
    
    float Shadow = 0;
    
#define SHADOW_QUALITY 3
        
#if SHADOW_QUALITY == 0
        Shadow = 1;

#elif SHADOW_QUALITY == 1
    Shadow = ShadowmapTexture.SampleCmpLevelZero(Sampler, ShadowmapUV, CompareDistance + ShadowDepthBias);

#elif SHADOW_QUALITY == 2
        
        [unroll]
        for (int i = 0; i < 5; ++i)
        {
            float2 SampleUV = ShadowmapUV + SideVector * DiscSamples5[i].x + UpVector * DiscSamples5[i].y;
            Shadow += ShadowmapTexture.SampleCmp(Sampler, SampleUV, CompareDistance + ShadowDepthBias * length(DiscSamples5[i]));
        }
        Shadow /= 5;
        
#elif SHADOW_QUALITY == 3
        
    [unroll]
    for (int i = 0; i < 12; ++i)
    {
        float2 SampleUV = ShadowmapUV + SideVector * DiscSamples12[i].x + UpVector * DiscSamples12[i].y;
        Shadow += ShadowmapTexture.SampleCmp(Sampler, SampleUV, CompareDistance + ShadowDepthBias * length(DiscSamples12[i]));
    }
    Shadow /= 12;

#elif SHADOW_QUALITY == 4
        [unroll]
        for (int i = 0; i < 29; ++i)
        {
            float2 SampleUV = ShadowmapUV + SideVector * DiscSamples29[i].x + UpVector * DiscSamples29[i].y;
            Shadow += ShadowmapTexture.SampleCmp(Sampler, SampleUV, CompareDistance + ShadowDepthBias * length(DiscSamples29[i]));
        }
        Shadow /= 29;

#endif
    
#undef SHADOW_QUALITY
    
    return Shadow;
}

float CalculateDirectionalLightShadow(float3 WorldPosition, float Depth, DirectionalLightData LightData, DirectionalLightShadowData ShadowData, SamplerComparisonState Sampler)
{
    Texture2DArray ShadowmapTexture = ResourceDescriptorHeap[ShadowData.ShadowMapTextureIndex];
    
    // TODO: seam blending
    float Alpha = 0;
    int index = 0;
    float CascadeDistance;
    for (uint i = 0; i < ShadowData.CascadeCount; i++)
    {
        CascadeDistance = ShadowData.CsSplitDistances[i / 4][i % 4];
        if (Depth < CascadeDistance)
        {
            index = i;
            break;
        }
    }
    
    float4 ShadowPos = mul(ShadowData.CsWorldToProjectionMatrices[index], float4(WorldPosition, 1));

    float DepthBias = ShadowData.DepthBias * ShadowData.CsSplitDistances[0][0] / CascadeDistance;
    
    float CompareDistance = ShadowPos.z / ShadowPos.w;
    float ShadowDepthBias = -DepthBias / ShadowPos.w;
    float2 ShadowmapUV = ShadowPos.xy / ShadowPos.w;
    ShadowmapUV = ShadowmapUV * 0.5f + 0.5f;
    ShadowmapUV.y = 1 - ShadowmapUV.y;

    float2 SideVector = mul(ShadowData.CsWorldToProjectionMatrices[index], float4(LightData.Direction, 0)).xy;
    SideVector = normalize(SideVector);
    float2 UpVector = normalize(float2(SideVector.y, -SideVector.x));
    
    //float2 SideVector = float2(1, 0);
    //float2 UpVector = float2(0, 1);
    
    SideVector *= ShadowData.InvShadowmapResolution;
    UpVector *= ShadowData.InvShadowmapResolution;
    
    SideVector /= asuint(1) << index;
    UpVector /= asuint(1) << index;
    
    float Shadow = 0;
    
    Shadow = ShadowmapTexture.SampleCmpLevelZero(Sampler, float3(ShadowmapUV, index), CompareDistance + ShadowDepthBias);
    
    #define SHADOW_QUALITY 3
        
#if SHADOW_QUALITY == 0
        Shadow = 1;

#elif SHADOW_QUALITY == 1
    Shadow = ShadowmapTexture.SampleCmpLevelZero(Sampler, float3(ShadowmapUV, index), CompareDistance + ShadowDepthBias);

#elif SHADOW_QUALITY == 2
        
        [unroll]
        for (int i = 0; i < 5; ++i)
        {
            float2 SampleUV = ShadowmapUV + SideVector * DiscSamples5[i].x + UpVector * DiscSamples5[i].y;
            Shadow += ShadowmapTexture.SampleCmpLevelZero(Sampler, float3(SampleUV, index), CompareDistance + ShadowDepthBias * length(DiscSamples5[i]));
        }
        Shadow /= 5;
        
#elif SHADOW_QUALITY == 3
        
    [unroll]
    for (int i = 0; i < 12; ++i)
    {
        float2 SampleUV = ShadowmapUV + SideVector * DiscSamples12[i].x + UpVector * DiscSamples12[i].y;
        Shadow += ShadowmapTexture.SampleCmpLevelZero(Sampler, float3(SampleUV, index), CompareDistance + ShadowDepthBias * length(DiscSamples12[i]));
    }
    Shadow /= 12;

#elif SHADOW_QUALITY == 4
        [unroll]
        for (int i = 0; i < 29; ++i)
        {
            float2 SampleUV = ShadowmapUV + SideVector * DiscSamples29[i].x + UpVector * DiscSamples29[i].y;
            Shadow += ShadowmapTexture.SampleCmp(Sampler, float3(SampleUV, index), CompareDistance + ShadowDepthBias * length(DiscSamples29[i]));
        }
        Shadow /= 29;

#endif
    
#undef SHADOW_QUALITY
    
    return Shadow;
}

uint ComputeLightGridCellIndex(LightGridData LightGrid, uint2 PixelPos, float SceneDepth)
{
    uint ZSlice = (uint) (max(0, log2(SceneDepth * LightGrid.LightGridZParams.x + LightGrid.LightGridZParams.y) * LightGrid.LightGridZParams.z));
    ZSlice = min(ZSlice, (uint) (LightGrid.CulledGridSize.z - 1));
    
    uint3 GridCoordinate = uint3(PixelPos >> LightGrid.LightGridPixelSizeShift, ZSlice);
    uint GridIndex = (GridCoordinate.z * LightGrid.CulledGridSize.y + GridCoordinate.y) * LightGrid.CulledGridSize.x + GridCoordinate.x;
    return GridIndex;
}

float3 CalculateLightingForTranslucency(ViewBuffer View, LightGridData LightGrid, GBufferData Gbuffer, float3 WorldPos, uint2 PixelPosition, float PixelDepth)
{
    float3 Result = float3(0, 0, 0);
    float3 CameraVector = View.CameraPos - WorldPos.xyz;
    
    [branch]
    if (LightGrid.HasDirectionalLight > 0)
    {
        DirectionalLightData Light;
        Light.Color = LightGrid.DirectionalLightColor;
        Light.Direction = LightGrid.DirectionalLightDirection;
        Light.ShadowDataIndex = 0;
        
        float3 Radiance = CalculateDirectionalLightRadiance(WorldPos.xyz, Light.Direction, Light.Color, CameraVector, Gbuffer);
        Result += Radiance * Gbuffer.AmbientOcclusion;
    }
    
    ConstantBuffer<LightGridPackedLocalLightData> PackedLocalLights = ResourceDescriptorHeap[LightGrid.LocalLightBufferIndex];

    Buffer<uint> LightGridNumOffset = ResourceDescriptorHeap[LightGrid.LightGridNumOffsetIndex];
    Buffer<uint> LightGridLinkList = ResourceDescriptorHeap[LightGrid.LightGridLinkListIndex];

    uint GridIndex = ComputeLightGridCellIndex(LightGrid, PixelPosition, PixelDepth);
    uint NumCulledLights = LightGridNumOffset[GridIndex * LIGHT_GRID_LINK_STRIDE + 0];
    uint LinkStart = LightGridNumOffset[GridIndex * LIGHT_GRID_LINK_STRIDE + 1];
    uint LinkEnd = LinkStart + NumCulledLights;
    
    [loop]
    for (uint Index = LinkStart; Index < LinkEnd; Index++)
    {
        uint LightIndex = LightGridLinkList[Index];
    
        LightGridLocalLightData LocalLightData = GetLocalLightData(PackedLocalLights, LightIndex);
        uint LightType = asuint(LocalLightData.LightDirectionAndLightType.w);

        [branch]
        if (LightType == LIGHT_GRID_LIGHT_TYPE_POINT)
        {
            PointLightData Light;
            Light.WorldPosAndScale = float4(LocalLightData.LightPositionAndInvRadius.xyz, LocalLightData.SpotAnglesAndSourceRadiusPacked.z);
            Light.Color = LocalLightData.LightColorAndFalloffExponent.rgb;
            Light.InvRadius = LocalLightData.LightPositionAndInvRadius.a;
            Light.ShadowDataIndex = 0;
            
            float3 Radiance = CalculatePointLightRadiance(WorldPos.xyz, Light.WorldPosAndScale.xyz, Light.Color, CameraVector, Gbuffer);
            float Attenuation = CalculatePointLightAttenuation(WorldPos.xyz, Light.WorldPosAndScale.xyz, Light.InvRadius);
            Result += Radiance * Attenuation * Gbuffer.AmbientOcclusion;
        }
        
        else if (LightType == LIGHT_GRID_LIGHT_TYPE_SPOT)
        {
            SpotLightData Light;
            Light.WorldPosition = LocalLightData.LightPositionAndInvRadius.xyz;
            Light.InvRadius = LocalLightData.LightPositionAndInvRadius.w;
            Light.Color = LocalLightData.LightColorAndFalloffExponent.rgb;
            Light.Direction = LocalLightData.LightDirectionAndLightType.xyz;
            Light.CosOuterCone = LocalLightData.SpotAnglesAndSourceRadiusPacked.x;
            Light.InvCosConeDifference = LocalLightData.SpotAnglesAndSourceRadiusPacked.y;
            Light.Attenution = LocalLightData.SpotAnglesAndSourceRadiusPacked.z;
            Light.InnerRadius = Light.OuterRadius = 0;
            Light.ShadowDataIndex = 0;
            
            float3 Radiance = CalculatePointLightRadiance(WorldPos.xyz, Light.WorldPosition, Light.Color, CameraVector, Gbuffer);
            float Attenuation = CalculateSpotLightAttenuation(WorldPos.xyz, Light.WorldPosition, Light.InvRadius, Light.Direction, Light.CosOuterCone, Light.InvCosConeDifference);
            Result += Radiance * Attenuation * Gbuffer.AmbientOcclusion;
        }
    }
    
    return Result;
}

// ---------------------------------------------------------------------------------------------------------------------------------------------

float3 ComputeF0(float3 BaseColor, float Metallic)
{
    //float3 F0 = float3(0.04, 0.04, 0.04);
    float3 F0 = 0.04;
    F0 = lerp(F0, BaseColor, Metallic);
    return F0;
}

float3 GetOffSpecularPeakReflectionDir(float3 Normal, float3 ReflectionVector, float Roughness)
{
    float a = Square(Roughness);
    return lerp(Normal, ReflectionVector, (1 - a) * (sqrt(1 - a) + a));
}

float GetSpecularOcclusion(float NoV, float RoughnessSq, float AO)
{
    return saturate(pow(NoV + AO, RoughnessSq) - 1 + AO);
}

float3 EnvBRDF(float3 SpecularColor, float Roughness, float NoV, Texture2D PreIntegratedGF, SamplerState State)
{
    float2 AB = PreIntegratedGF.Sample(State, float2(NoV, Roughness)).rg;
    float3 GF = SpecularColor * AB.x + saturate(50.0 * SpecularColor.g) * AB.y;
    return GF;
}

half2 EnvBRDFApproxLazarov(half Roughness, half NoV)
{
    const half4 c0 = { -1, -0.0275, -0.572, 0.022 };
    const half4 c1 = { 1, 0.0425, 1.04, -0.04 };
    half4 r = Roughness * c0 + c1;
    half a004 = min(r.x * r.x, exp2(-9.28 * NoV)) * r.x + r.y;
    half2 AB = half2(-1.04, 1.04) * a004 + r.zw;
    return AB;
}

half3 EnvBRDFApprox(half3 SpecularColor, half Roughness, half NoV)
{
    half2 AB = EnvBRDFApproxLazarov(Roughness, NoV);
    float F90 = saturate(50.0 * SpecularColor.g);

    return SpecularColor * AB.x + F90 * AB.y;
}

half EnvBRDFApproxNonmetal(half Roughness, half NoV)
{
    const half2 c0 = { -1, -0.0275 };
    const half2 c1 = { 1, 0.0425 };
    half2 r = Roughness * c0 + c1;
    return min(r.x * r.x, exp2(-9.28 * NoV)) * r.x + r.y;
}

void EnvBRDFApproxFullyRough(inout half3 DiffuseColor, inout half3 SpecularColor)
{
    DiffuseColor += SpecularColor * 0.45;
    SpecularColor = 0;
}
void EnvBRDFApproxFullyRough(inout half3 DiffuseColor, inout half SpecularColor)
{
    DiffuseColor += SpecularColor * 0.45;
    SpecularColor = 0;
}

float3 fresnelSchlickRoughness(float cosTheta, float3 F0, float roughness)
{
    float a = 1.0 - roughness;
    return F0 + (max(float3(a.xxx), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float3 GetLookupVectorForSphereCapture(float3 ReflectionVector, float3 WorldPosition, float4 SphereCapturePositionAndRadius, float NormalizedDistanceToCapture, float3 LocalCaptureOffset, inout float DistanceAlpha)
{
    float3 ProjectedCaptureVector = ReflectionVector;
    float ProjectionSphereRadius = SphereCapturePositionAndRadius.w;
    float SphereRadiusSquared = ProjectionSphereRadius * ProjectionSphereRadius;

    float3 LocalPosition = WorldPosition - SphereCapturePositionAndRadius.xyz;
    float LocalPositionSqr = dot(LocalPosition, LocalPosition);

	// Find the intersection between the ray along the reflection vector and the capture's sphere
    float3 QuadraticCoef;
    QuadraticCoef.x = 1;
    QuadraticCoef.y = dot(ReflectionVector, LocalPosition);
    QuadraticCoef.z = LocalPositionSqr - SphereRadiusSquared;

    float Determinant = QuadraticCoef.y * QuadraticCoef.y - QuadraticCoef.z;

	// Only continue if the ray intersects the sphere
	[flatten]
    if (Determinant >= 0)
    {
        float FarIntersection = sqrt(Determinant) - QuadraticCoef.y;

        float3 LocalIntersectionPosition = LocalPosition + FarIntersection * ReflectionVector;
        ProjectedCaptureVector = LocalIntersectionPosition - LocalCaptureOffset;
		// Note: some compilers don't handle smoothstep min > max (this was 1, .6)
		//DistanceAlpha = 1.0 - smoothstep(.6, 1, NormalizedDistanceToCapture);

        float x = saturate(2.5 * NormalizedDistanceToCapture - 1.5);
        DistanceAlpha = 1 - x * x * (3 - 2 * x);
    }
    return ProjectedCaptureVector;
}

float3 GetEnvironemntReflection(ViewBuffer View, LightGridData LightGrid, GBufferData Gbuffer, float4 SSR, float3 WorldPosition, uint2 PixelPosition, float PixelDepth, SamplerState LinearClampSampler)
{
    Texture2D PreintegeratedGFImage = ResourceDescriptorHeap[LightGrid.PreintegeratedGFImageIndex];
    
    float3 DiffueColor = Gbuffer.BaseColor - Gbuffer.BaseColor * Gbuffer.Matallic;
    float3 SpecularColor = ComputeF0(Gbuffer.BaseColor, Gbuffer.Matallic);
    
    float3 CameraToPixel = normalize(WorldPosition - View.CameraPos);
    float3 ReflectionVector = reflect(CameraToPixel, Gbuffer.WorldNormal);
        
    float3 N = Gbuffer.WorldNormal;
    float3 V = -CameraToPixel;
    float NoV = saturate(dot(N, V));
        
    //float3 R = 2 * dot(V, N) * N - V;
    //R = GetOffSpecularPeakReflectionDir(N, R, Roughness);
        
    //float4 SSR = SSRImage.Sample(PointClampSampler, UV);
    float3 SpecularTerm = SSR.rgb;

    float RoughnessSq = Square(Gbuffer.Roughness);
    float SpecularOcclusion = GetSpecularOcclusion(NoV, RoughnessSq, Gbuffer.AmbientOcclusion);

    float3 LightDiffuse = LightGrid.SkyLightColor;
        
    float3 F = fresnelSchlickRoughness(max(dot(Gbuffer.WorldNormal, -CameraToPixel), 0.0), SpecularColor, Gbuffer.Roughness);
    float3 kS = F;
    float3 kD = 1 - kS;
    kD *= 1.0f - Gbuffer.Matallic;
    
    //half Mip = ComputeReflectionCaptureMipFromRoughness(Gbuffer.Roughness, 8); // TODO: pass mip count as constant
    half Mip = ComputeReflectionCaptureMipFromRoughness(RoughnessSq, 8);
    float4 ImageBasedReflections = float4(0, 0, 0, 1.0f);
    float3 RayDirection = ReflectionVector;
    
    
    ConstantBuffer<LightGridPackedReflectionCaptureData> PackedReflectionCaptures = ResourceDescriptorHeap[LightGrid.ReflectionCaptureBufferIndex];

    Buffer<uint> LightGridNumOffset = ResourceDescriptorHeap[LightGrid.LightGridNumOffsetIndex];
    Buffer<uint> LightGridLinkList = ResourceDescriptorHeap[LightGrid.LightGridLinkListIndex];

    uint GridIndex = ComputeLightGridCellIndex(LightGrid, PixelPosition, PixelDepth);
    uint NumCulledLights = LightGridNumOffset[(LightGrid.NumGridCells + GridIndex) * LIGHT_GRID_LINK_STRIDE + 0];
    uint LinkStart = LightGridNumOffset[(LightGrid.NumGridCells + GridIndex) * LIGHT_GRID_LINK_STRIDE + 1];
    uint LinkEnd = LinkStart + NumCulledLights;
    
    [loop]
    for (uint Index = LinkStart; Index < LinkEnd; Index++)
    {
        uint ReflectionCaptureIndex = LightGridLinkList[Index];
        LightGridReflectionCaptureData ReflectionCaptureData = GetReflectionCaptureData(PackedReflectionCaptures, ReflectionCaptureIndex);

        if (ImageBasedReflections.a < 0.001f)
        {
            break;
        }
        
        float4 CapturePositionAndRadius = ReflectionCaptureData.PositionAndRadius;
        float3 CaptureVector = WorldPosition - CapturePositionAndRadius.xyz;
        float CaptureVectorLength = sqrt(dot(CaptureVector, CaptureVector));
        float NormalizedDistanceToCapture = saturate(CaptureVectorLength / CapturePositionAndRadius.w);
        
        // apply smaller first. already sorted in increasing size
        [branch]
        if (CaptureVectorLength < CapturePositionAndRadius.w)
        {
            float3 ProjectedCaptureVector = RayDirection;
            float4 CaptureOffsetAndAverageBrightness = ReflectionCaptureData.CaptureOffsetBrightness;
            uint CubemapIndex = ReflectionCaptureData.CubemapIndex;

            float DistanceAlpha = 0;
			
            ProjectedCaptureVector = GetLookupVectorForSphereCapture(RayDirection, WorldPosition, CapturePositionAndRadius, NormalizedDistanceToCapture, CaptureOffsetAndAverageBrightness.xyz, DistanceAlpha);

			{
                TextureCube ReflectionCubemap = ResourceDescriptorHeap[CubemapIndex];
                float4 Sample = ReflectionCubemap.SampleLevel(LinearClampSampler, ProjectedCaptureVector, Mip);
                
				//Sample.rgb *= CaptureProperties.r;
                Sample *= DistanceAlpha;
                
				// Under operator (back to front)
                ImageBasedReflections.rgb += Sample.rgb * ImageBasedReflections.a * SpecularOcclusion;
                ImageBasedReflections.a *= 1 - Sample.a;

				//float AverageBrightness = CaptureOffsetAndAverageBrightness.w;
				//CompositedAverageBrightness.x += AverageBrightness * DistanceAlpha * CompositedAverageBrightness.y;
				//CompositedAverageBrightness.y *= 1 - DistanceAlpha;
            }
        }
    }
    
    [branch]
    if (LightGrid.HasSkyLight)
    {
        TextureCube SkyConvolution = ResourceDescriptorHeap[LightGrid.SkyLightConvolutionIndex];
        TextureCube SkyIrradiance = ResourceDescriptorHeap[LightGrid.SkyLightIrradianceIndex];
        
        half MipLevel = ComputeReflectionCaptureMipFromRoughness(Gbuffer.Roughness, LightGrid.SkyLightMipCount);
        //half MipLevel = ComputeReflectionCaptureMipFromRoughness(RoughnessSq, SSRBuffer.SkyLightMipCount);
        ImageBasedReflections.rgb += ImageBasedReflections.a * LightGrid.SkyLightColor * SkyConvolution.SampleLevel(LinearClampSampler, ReflectionVector, MipLevel).xyz;
        
        LightDiffuse *= SkyIrradiance.SampleLevel(LinearClampSampler, Gbuffer.WorldNormal, 0).rgb;
    }
    
    float3 DiffuseTerm = LightDiffuse * kD * DiffueColor;

    ImageBasedReflections.rgb *= SpecularOcclusion;
    SpecularTerm += ImageBasedReflections.rgb;
    float3 BRDF = EnvBRDF(SpecularColor, Gbuffer.Roughness, NoV, PreintegeratedGFImage, LinearClampSampler);
    
    SpecularTerm *= BRDF;
    //return float4(SpecularTerm, 1);
    //return float4( DiffuseTerm * CombinedAO, 1);
    return SpecularTerm + DiffuseTerm * Gbuffer.AmbientOcclusion;
}