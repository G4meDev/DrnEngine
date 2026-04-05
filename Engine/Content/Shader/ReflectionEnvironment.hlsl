#include "Common.hlsl"

#define MAX_REFLECTION_CAPTURE_COUNT 32

struct Resources
{
    uint ViewBufferIndex;
    uint SSRBufferIndex;
    uint StaticSamplerBufferIndex;
};

ConstantBuffer<Resources> BindlessResources : register(b0);

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