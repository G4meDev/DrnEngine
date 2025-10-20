#include "Common.hlsl"

#define LIGHT_BITFLAG_POINTLIGHT 1
#define LIGHT_BITFLAG_SPOTLIGHT 2
#define LIGHT_BITFLAG_DIRECTIONAL 4
#define LIGHT_BITFLAG_SKY 8

#ifndef SPOTLIGHT_STENCIL_SIDES
#define SPOTLIGHT_STENCIL_SIDES 18
#endif

#ifndef SPOTLIGHT_STENCIL_SLICES
#define SPOTLIGHT_STENCIL_SLICES 12
#endif

float2 VSPosToScreenUV(float4 VSPos)
{
    float2 UV = VSPos.xy / VSPos.w;
    UV = UV / 2 + 0.5f;
    UV.y = 1 - UV.y;
    
    return UV;
}

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

// --------------------------------------------------------------------------

struct Resources
{
    uint ViewBufferIndex;
    uint LightDataIndex;
    uint StaticSamplerBufferIndex;
    uint BaseColorIndex;
    uint WorldNormalIndex;
    uint MasksIndex;
    uint DepthIndex;
    uint LightFlags;
    uint SSAOIndex;
    uint MasksBIndex;
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
};

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

struct StaticSamplers
{
    uint LinearSamplerIndex;
    uint PointSamplerIndex;
    uint LinearCompLessSamplerIndex;
};

struct VertexInputPosUV
{
    float3 Position : POSITION;
};

struct VertexShaderOutput
{
    noperspective float2 UV : TEXCOORD;
    noperspective float2 ScreenPos : TEXCOORD1;
    float4 Position : SV_Position;
};

VertexShaderOutput Main_VS(VertexInputPosUV IN, uint InVertexId : SV_VertexID)
{
    VertexShaderOutput OUT;
    ConstantBuffer<ViewBuffer> View = ResourceDescriptorHeap[BindlessResources.ViewBufferIndex];
    float3 WorldPosition;

    [branch]
    if (BindlessResources.LightFlags & LIGHT_BITFLAG_POINTLIGHT)
    {
        ConstantBuffer<PointLightData> LightBuffer = ResourceDescriptorHeap[BindlessResources.LightDataIndex];
        WorldPosition = IN.Position * LightBuffer.WorldPosAndScale.w + LightBuffer.WorldPosAndScale.xyz;
        OUT.Position = mul(View.WorldToProjection, float4(WorldPosition, 1.0f));
    }

    else if(BindlessResources.LightFlags & LIGHT_BITFLAG_SPOTLIGHT)
    {
        ConstantBuffer<SpotLightData> LightBuffer = ResourceDescriptorHeap[BindlessResources.LightDataIndex];
        
        float SphereRadius = LightBuffer.Attenution;
        float ConeAngle = LightBuffer.OuterRadius;

        uint NumSides = SPOTLIGHT_STENCIL_SIDES;
        uint NumSlices = SPOTLIGHT_STENCIL_SLICES;
        
        const float InvCosRadiansPerSide = 1.0f / cos(PI / (float) NumSides);
        const float ZRadius = SphereRadius * cos(ConeAngle);
        const float TanConeAngle = tan(ConeAngle);

        float3 LocalPosition = float3(1, 1, 1);
        
        uint CapIndexStart = NumSides * NumSlices;
        if (InVertexId < CapIndexStart)
        {
            uint SideIndex = InVertexId / NumSlices;
            uint SliceIndex = InVertexId % NumSlices;
            
            const float CurrentAngle = SideIndex * 2 * PI / (float) NumSides;
            const float DistanceDownConeDirection = ZRadius * SliceIndex / (float) (NumSlices - 1);
            const float SliceRadius = DistanceDownConeDirection * TanConeAngle * InvCosRadiansPerSide;
            LocalPosition = float3(SliceRadius * cos(CurrentAngle), SliceRadius * sin(CurrentAngle), ZRadius * SliceIndex / (float) (NumSlices - 1));
            //WorldPosition = mul(float4(LocalPosition, 1), StencilingConeTransform).xyz + StencilingPreViewTranslation;
        }
        else
        {
            const float CapRadius = ZRadius * tan(ConeAngle);

            uint VertexId = InVertexId - CapIndexStart;

            uint SideIndex = VertexId / NumSlices;
            uint SliceIndex = VertexId % NumSlices;
            
            const float UnadjustedSliceRadius = CapRadius * SliceIndex / (float) (NumSlices - 1);
            const float SliceRadius = UnadjustedSliceRadius * InvCosRadiansPerSide;
            const float ZDistance = sqrt(SphereRadius * SphereRadius - UnadjustedSliceRadius * UnadjustedSliceRadius);

            const float CurrentAngle = SideIndex * 2 * PI / (float) NumSides;
            LocalPosition = float3(SliceRadius * cos(CurrentAngle), SliceRadius * sin(CurrentAngle), ZDistance);
            //WorldPosition = mul(float4(LocalPosition, 1), StencilingConeTransform).xyz + StencilingPreViewTranslation;
        }
        
        WorldPosition = mul(LightBuffer.LocalToWorld, float4(LocalPosition, 1)).xyz;
        OUT.Position = mul(View.WorldToProjection, float4(WorldPosition, 1.0f));
    }

    else if (BindlessResources.LightFlags & (LIGHT_BITFLAG_DIRECTIONAL))
    {
        OUT.Position = mul(View.LocalToCameraView, float4(IN.Position, 1.0f));
        OUT.Position.z = 0;
    }
    
    else
    {
        WorldPosition = float3(1, 1, 1);
    }

    OUT.UV = VSPosToScreenUV(OUT.Position);
    OUT.ScreenPos = OUT.Position.xy / OUT.Position.w;
    
    return OUT;
}

struct PixelShaderInput
{
    noperspective float2 UV : TEXCOORD;
    noperspective float2 ScreenPos : TEXCOORD1;
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

static const float2 DiscSamples29[]=
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

float ConvertFromDeviceZ(float DeviceZ, float4 InvDeviceZToWorldZTransform)
{
    return DeviceZ * InvDeviceZToWorldZTransform[0] + InvDeviceZToWorldZTransform[1] + 1.0f / (DeviceZ * InvDeviceZToWorldZTransform[2] - InvDeviceZToWorldZTransform[3]);
}

float3 CalculatePointLightRadiance(float3 WorldPosition, float3 LightPosition, float3 LightColor, float3 CameraVector, GBufferData Gbuffer)
{
    float3 ToLight = LightPosition - WorldPosition;
    float DistanceSquare = dot(ToLight, ToLight);
    float3 L = ToLight * rsqrt(DistanceSquare);
    
    [branch]
    if (Gbuffer.ShadingModel == SHADING_MODEL_FOLIAGE && dot(Gbuffer.WorldNormal, -L) > 0.0f)
    {
        return CalculateLighting(L, -Gbuffer.WorldNormal, normalize(CameraVector), LightColor, Gbuffer.TransmittanceColor, Gbuffer.Matallic, Gbuffer.Roughness);
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
        return CalculateLighting(-LightDirection, -Gbuffer.WorldNormal, normalize(CameraVector), LightColor, Gbuffer.TransmittanceColor, Gbuffer.Matallic, Gbuffer.Roughness);
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

float CalculateSpotLightAttenuation(float3 WorldPosition, float3 LightPosition, float InvRadius, float3 Direction, float CosOuterCone, float CosInnerCone, float InvCosConeDifference)
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

float4 Main_PS(PixelShaderInput IN) : SV_Target
{
    ConstantBuffer<ViewBuffer> View = ResourceDescriptorHeap[BindlessResources.ViewBufferIndex];
    
    ConstantBuffer<StaticSamplers> StaticSamplers = ResourceDescriptorHeap[BindlessResources.StaticSamplerBufferIndex];
    SamplerState LinearSampler = ResourceDescriptorHeap[StaticSamplers.LinearSamplerIndex];
    
    Texture2D MasksTexture = ResourceDescriptorHeap[BindlessResources.MasksIndex];
    float4 Masks = MasksTexture.Sample(LinearSampler, IN.UV);
    
    uint ShadingModel = FloatToUint8(Masks.a);
    [branch]
    if (ShadingModel == SHADING_MODEL_UNLIT)
        return 0;
    
    Texture2D BaseColorTexture = ResourceDescriptorHeap[BindlessResources.BaseColorIndex];
    Texture2D WorldNormalTexture = ResourceDescriptorHeap[BindlessResources.WorldNormalIndex];
    Texture2D MasksBTexture = ResourceDescriptorHeap[BindlessResources.MasksBIndex];
    Texture2D DepthTexture = ResourceDescriptorHeap[BindlessResources.DepthIndex];
    Texture2D SSAOTexture = ResourceDescriptorHeap[BindlessResources.SSAOIndex];
    
    float4 BaseColor = BaseColorTexture.Sample(LinearSampler, IN.UV);
    float3 Transmittance = MasksBTexture.Sample(LinearSampler, IN.UV).rgb;
    float2 WorldNormal = WorldNormalTexture.Sample(LinearSampler, IN.UV).xy;
    float Depth = DepthTexture.Sample(LinearSampler, IN.UV).x;
    
    
    //float4 WorldPos = mul(View.ProjectionToWorld, float4(IN.ScreenPos, Depth, 1));
    //WorldPos.xyz /= WorldPos.w;
    float EE = ConvertFromDeviceZ(Depth, View.InvDeviceZToWorldZTransform);
    float4 WorldPos = mul(View.ScreenToTranslatedWorld, float4(IN.ScreenPos * EE, EE, 1));
    
    float3 N = DecodeNormal(WorldNormal);
    
    GBufferData Gbuffer;
    Gbuffer.BaseColor = BaseColor.xyz;
    Gbuffer.WorldNormal = N;
    Gbuffer.Matallic = Masks.r;
    Gbuffer.Roughness = Masks.g;
    Gbuffer.AmbientOcclusion = Masks.b;
    Gbuffer.TransmittanceColor = Transmittance;
    Gbuffer.ShadingModel = ShadingModel;
    
    float3 CameraVector = View.CameraPos - WorldPos.xyz;
    float3 Radiance = 0;
    float Attenuation = 1;
    float Shadow = 1;
    
    float SSAO = SSAOTexture.Sample(LinearSampler, IN.UV).x;
    float CombinedAO = SSAO * Masks.b;
    
    [branch]
    if (BindlessResources.LightFlags & LIGHT_BITFLAG_POINTLIGHT)
    {
        ConstantBuffer<PointLightData> Light = ResourceDescriptorHeap[BindlessResources.LightDataIndex];
        Radiance = CalculatePointLightRadiance(WorldPos.xyz, Light.WorldPosAndScale.xyz, Light.Color, CameraVector, Gbuffer);
        Attenuation = CalculatePointLightAttenuation(WorldPos.xyz, Light.WorldPosAndScale.xyz, Light.InvRadius);
        
        [branch]
        if(Light.ShadowDataIndex != 0)
        {
            ConstantBuffer<PointLightShadowData> ShadowBuffer = ResourceDescriptorHeap[Light.ShadowDataIndex];
            TextureCube ShadowmapTexture = ResourceDescriptorHeap[ShadowBuffer.ShadowMapTextureIndex];
            SamplerComparisonState CompState = ResourceDescriptorHeap[StaticSamplers.LinearCompLessSamplerIndex];
            Shadow = CalculatePointLightShadow(WorldPos.xyz, Light.WorldPosAndScale.xyz, ShadowBuffer.WorldToProjectionMatrices,
                ShadowBuffer.DepthBias, ShadowBuffer.InvShadowmapResolution, ShadowmapTexture, CompState);
        }
    }
    
    else if(BindlessResources.LightFlags & LIGHT_BITFLAG_SPOTLIGHT)
    {
        ConstantBuffer<SpotLightData> Light = ResourceDescriptorHeap[BindlessResources.LightDataIndex];
        Attenuation = CalculateSpotLightAttenuation(WorldPos.xyz, Light.WorldPosition, Light.InvRadius, Light.Direction, Light.CosOuterCone, Light.InnerRadius, Light.InvCosConeDifference);
        Radiance = CalculatePointLightRadiance(WorldPos.xyz, Light.WorldPosition, Light.Color, CameraVector, Gbuffer);
        
        [branch]
        if(Light.ShadowDataIndex != 0)
        {
            ConstantBuffer<SpotLightShadowData> ShadowBuffer = ResourceDescriptorHeap[Light.ShadowDataIndex];
            Texture2D ShadowmapTexture = ResourceDescriptorHeap[ShadowBuffer.ShadowMapTextureIndex];
            SamplerComparisonState CompState = ResourceDescriptorHeap[StaticSamplers.LinearCompLessSamplerIndex];
            float3 ToLight = Light.WorldPosition - WorldPos.xyz;
            Shadow = CalculateSpotLightShadow(WorldPos.xyz, ToLight, ShadowBuffer.WorldToProjectionMatrix,
                ShadowBuffer.DepthBias, ShadowBuffer.InvShadowmapResolution, ShadowmapTexture, CompState);
        }
    }
    
    else if (BindlessResources.LightFlags & LIGHT_BITFLAG_DIRECTIONAL)
    {
        [branch]
        if (Depth > 0.0001)
        {
            ConstantBuffer<DirectionalLightData> Light = ResourceDescriptorHeap[BindlessResources.LightDataIndex];
            Radiance = CalculateDirectionalLightRadiance(WorldPos.xyz, Light.Direction, Light.Color, CameraVector, Gbuffer);
            
            [branch]
            if (Light.ShadowDataIndex != 0)
            {
                ConstantBuffer<DirectionalLightShadowData> ShadowBuffer = ResourceDescriptorHeap[Light.ShadowDataIndex];
                SamplerComparisonState CompState = ResourceDescriptorHeap[StaticSamplers.LinearCompLessSamplerIndex];
                Shadow = CalculateDirectionalLightShadow(WorldPos.xyz, ConvertFromDeviceZ(Depth, View.InvDeviceZToWorldZTransform), Light, ShadowBuffer, CompState);
            }
        }
    }
    
    return float4(Radiance * Attenuation * Shadow * CombinedAO, 1);
}