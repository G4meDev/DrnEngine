#include "Common.hlsl"

#define MAX_REFLECTION_CAPTURE_COUNT 32

struct Resources
{
    uint ViewBufferIndex;
    uint SSRBufferIndex;
    uint StaticSamplerBufferIndex;
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

struct ReflectionCaptureData
{
    uint ReflectionTexture;
    float3 Padding;
    
    float4 PositionRadius;
    float4 OffsetBrightness;
};

struct SSRData
{
    uint BaseColorTexture;
    uint WorldNormalTexture;
    uint MasksTexture;
    uint DepthTexture;
    
    uint SSRTexture;
    uint AOTexture;
    uint PreintegratedGF;
    uint SkyCubemapTexture;
    
    float3 SkyLightColor;
    uint SkyLightMipCount;
    
    uint SkyIradianceCubemapTexture;
    uint NumReflectionCaptures;
    float2 Pad_1;

    ReflectionCaptureData CaptureData[MAX_REFLECTION_CAPTURE_COUNT];
};

struct StaticSamplers
{
    uint LinearSamplerIndex;
    uint PointSamplerIndex;
    uint LinearCmpSamplerIndex;
    uint LinearClampIndex;
    uint PointClampIndex;
};

struct VertexInputPosUV
{
    float3 Position : POSITION;
    float2 UV : TEXCOORD;
};

struct VertexShaderOutput
{
    float4 UVAndScreenPos : TEXCOORD0;
    float4 Position : SV_Position;
};

VertexShaderOutput Main_VS(VertexInputPosUV IN)
{
    VertexShaderOutput OUT;

    ConstantBuffer<ViewBuffer> View = ResourceDescriptorHeap[BindlessResources.ViewBufferIndex];
    
    OUT.Position = mul(View.LocalToCameraView, float4(IN.Position, 1.0f));
    OUT.Position.z = 0;
    OUT.UVAndScreenPos = float4(IN.UV, IN.Position.xy);
    
    return OUT;
}

struct PixelShaderInput
{
    float4 UVAndScreenPos : TEXCOORD0;
    float4 Position : SV_Position;
};

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
    float2 AB = PreIntegratedGF.Sample( State, float2(NoV, Roughness)).rg;
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

		float x = saturate( 2.5 * NormalizedDistanceToCapture - 1.5 );
		DistanceAlpha = 1 - x*x*(3 - 2*x);
	}
	return ProjectedCaptureVector;
}

float4 Main_PS(PixelShaderInput IN) : SV_Target
{
    ConstantBuffer<ViewBuffer> View = ResourceDescriptorHeap[BindlessResources.ViewBufferIndex];
    ConstantBuffer<SSRData> SSRBuffer = ResourceDescriptorHeap[BindlessResources.SSRBufferIndex];
    ConstantBuffer<StaticSamplers> StaticSamplersBuffer = ResourceDescriptorHeap[BindlessResources.StaticSamplerBufferIndex];
    
    Texture2D ColorImage = ResourceDescriptorHeap[SSRBuffer.BaseColorTexture];
    Texture2D NormalImage = ResourceDescriptorHeap[SSRBuffer.WorldNormalTexture];
    Texture2D MaskImage = ResourceDescriptorHeap[SSRBuffer.MasksTexture];
    Texture2D DepthImage = ResourceDescriptorHeap[SSRBuffer.DepthTexture];
    Texture2D SSRImage = ResourceDescriptorHeap[SSRBuffer.SSRTexture];
    Texture2D AOImage = ResourceDescriptorHeap[SSRBuffer.AOTexture];
    Texture2D PreintegeratedGFImage = ResourceDescriptorHeap[SSRBuffer.PreintegratedGF];
    TextureCube SkyCubemapImage = ResourceDescriptorHeap[SSRBuffer.SkyCubemapTexture];
    TextureCube SkyIradianceCubemapTexture = ResourceDescriptorHeap[SSRBuffer.SkyIradianceCubemapTexture];
    
    SamplerState PointSampler = ResourceDescriptorHeap[StaticSamplersBuffer.PointSamplerIndex];
    SamplerState PointClampSampler = ResourceDescriptorHeap[StaticSamplersBuffer.PointClampIndex];
    SamplerState LinearSampler = ResourceDescriptorHeap[StaticSamplersBuffer.LinearSamplerIndex];
    SamplerState LinearClampSampler = ResourceDescriptorHeap[StaticSamplersBuffer.LinearClampIndex];
    
    float2 UV = IN.UVAndScreenPos.xy;
    float2 ScreenPos = IN.UVAndScreenPos.zw;
    
    float3 BaseColor = ColorImage.Sample(PointClampSampler, UV).xyz;
    float4 Masks = MaskImage.Sample(PointClampSampler, UV);
    float Metallic = Masks.r;
    float Roughness = Masks.g;
    float AO = Masks.b;
    
    uint ShadingModel = FloatToUint8(Masks.a);
    [branch]
    if (ShadingModel == SHADING_MODEL_UNLIT)
        return 0;
    
    float2 EncodedNormal = NormalImage.Sample(PointClampSampler, UV).xy;
    float3 WorldNormal = DecodeNormal(EncodedNormal);
    float Depth = DepthImage.Sample(PointClampSampler, UV).x;
    float SceneDepth = ConvertFromDeviceZ(Depth, View.InvDeviceZToWorldZTransform);
    
    float SSAO = AOImage.Sample(PointClampSampler, UV).x;
    float3 DiffueColor = BaseColor - BaseColor * Metallic;
    float3 SpecularColor = ComputeF0(BaseColor, Metallic);

    float3 WorldPosition = mul(View.ScreenToTranslatedWorld, float4(ScreenPos * SceneDepth, SceneDepth, 1)).xyz;
        
    float3 CameraToPixel = normalize(WorldPosition - View.CameraPos);
    float3 ReflectionVector = reflect(CameraToPixel, WorldNormal);
        
    float3 N = WorldNormal;
    float3 V = -CameraToPixel;
    float NoV = saturate(dot(N, V));
        
    //float3 R = 2 * dot(V, N) * N - V;
    //R = GetOffSpecularPeakReflectionDir(N, R, Roughness);
        
    float4 SSR = SSRImage.Sample(PointClampSampler, UV);
    float3 SpecularTerm = SSR.rgb;
        
    float CombinedAO = AO * SSAO;
    float RoughnessSq = Square(Roughness);
    float SpecularOcclusion = GetSpecularOcclusion(NoV, RoughnessSq, CombinedAO);

    float3 LightDiffuse = SSRBuffer.SkyLightColor;
        
    float3 F = fresnelSchlickRoughness(max(dot(WorldNormal, -CameraToPixel), 0.0), SpecularColor, Roughness);
    float3 kS = F;
    float3 kD = 1 - kS;
    kD *= 1.0f - Metallic;
    
    half Mip = ComputeReflectionCaptureMipFromRoughness(Roughness, 8); // TODO: pass mip count as constant
    //float Mip = ComputeReflectionCaptureMipFromRoughness(RoughnessSq, 8);
    float4 ImageBasedReflections = float4(0, 0, 0, 1.0f);
    float3 RayDirection = ReflectionVector;
    
    for(uint CaptureIndex = 0; CaptureIndex < SSRBuffer.NumReflectionCaptures; CaptureIndex++)
    {
        if(ImageBasedReflections.a < 0.001f)
        {
            break;
        }
        
        float4 CapturePositionAndRadius = SSRBuffer.CaptureData[CaptureIndex].PositionRadius;
        float3 CaptureVector = WorldPosition - CapturePositionAndRadius.xyz;
        float CaptureVectorLength = sqrt(dot(CaptureVector, CaptureVector));
        float NormalizedDistanceToCapture = saturate(CaptureVectorLength / CapturePositionAndRadius.w);
        
        [branch]
		if (CaptureVectorLength < CapturePositionAndRadius.w)
		{
            float3 ProjectedCaptureVector = RayDirection;
            float4 CaptureOffsetAndAverageBrightness = SSRBuffer.CaptureData[CaptureIndex].OffsetBrightness;

			float DistanceAlpha = 0;
			
			ProjectedCaptureVector = GetLookupVectorForSphereCapture(RayDirection, WorldPosition, CapturePositionAndRadius, NormalizedDistanceToCapture, CaptureOffsetAndAverageBrightness.xyz, DistanceAlpha);

			{
                TextureCube ReflectionCubemap = ResourceDescriptorHeap[SSRBuffer.CaptureData[CaptureIndex].ReflectionTexture];
				float4 Sample = ReflectionCubemap.SampleLevel(LinearSampler, ProjectedCaptureVector, Mip);

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
    if(SSRBuffer.SkyCubemapTexture != 0)
    {
        half MipLevel = ComputeReflectionCaptureMipFromRoughness(Roughness, SSRBuffer.SkyLightMipCount);
        //half MipLevel = ComputeReflectionCaptureMipFromRoughness(RoughnessSq, SSRBuffer.SkyLightMipCount);
        ImageBasedReflections.rgb += ImageBasedReflections.a * SSRBuffer.SkyLightColor * SkyCubemapImage.SampleLevel(LinearSampler, ReflectionVector, MipLevel).xyz;
        
        LightDiffuse *= SkyIradianceCubemapTexture.SampleLevel(LinearSampler, WorldNormal, 0).rgb;
    }
    float3 DiffuseTerm = LightDiffuse * kD * DiffueColor;

    ImageBasedReflections.rgb *= (1 - SSR.a) * SpecularOcclusion;
    SpecularTerm += ImageBasedReflections.rgb;
    float3 BRDF = EnvBRDF(SpecularColor, Roughness, NoV, PreintegeratedGFImage, LinearClampSampler);
    
    SpecularTerm *= BRDF;
    //return float4(SpecularTerm, 1);
    //return float4( DiffuseTerm * CombinedAO, 1);
    return float4(SpecularTerm + DiffuseTerm * CombinedAO, 1);
}