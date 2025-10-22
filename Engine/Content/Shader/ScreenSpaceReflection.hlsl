
#include "Common.hlsl"

#define SAMPLESET_ARRAY_SIZE 6
#define SAMPLE_STEPS 3

#define PI 3.1415926535897932f
#define SSRT_SAMPLE_BATCH_SIZE 4

const static float Constant_Float16F_Scale = 4096.0f;

static const float2 OcclusionSamplesOffsets[SAMPLESET_ARRAY_SIZE] =
{
    float2(0.000, 0.200),
		float2(0.325, 0.101),
		float2(0.272, -0.396),
		float2(-0.385, -0.488),
		float2(-0.711, 0.274),
		float2(0.060, 0.900)
};

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
    float2 JitterOffset;
    
    float2 PrevJitterOffset;
    float2 Pad_1;
    
    matrix ClipToPreviousClip;
};

struct SSRData
{
    uint DeferredColorTexture;
    uint BaseColorTexture;
    uint WorldNormalTexture;
    uint MasksTexture;
    uint DepthTexture;
    uint HzbTexture;
    float Intensity;
    float RoughnessFade;
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

float GetRoughnessFade(float Roughness, float Fade)
{
    return min(Roughness * Fade + 2, 1.0);
    //return 1 - saturate(Roughness * 1.6f);
}

float Square(float x) { return x * x; }

//float3x3 GetTangentBasis(float3 TangentZ)
float3x3 GetTangentBasis(float3 TangentY)
{
    //const float Sign = TangentZ.z >= 0 ? 1 : -1;
    //const float a = -rcp(Sign + TangentZ.z);
    //const float b = TangentZ.x * TangentZ.y * a;
	//
    //float3 TangentX = { 1 + Sign * a * Square(TangentZ.x), Sign * b, -Sign * TangentZ.x };
    //float3 TangentY = { b, Sign + a * Square(TangentZ.y), -TangentZ.y };
    //
    //return float3x3(TangentX, TangentY, TangentZ);
    
    const float Sign = TangentY.y >= 0 ? 1 : -1;
    const float a = -rcp(Sign + TangentY.y);
    const float b = TangentY.x * TangentY.z * a;
	
    //float3 TangentX = { 1 + Sign * a * Square(TangentY.x), Sign * b, -Sign * TangentY.x };
    //float3 TangentZ = { b, Sign + a * Square(TangentY.z), -TangentY.z };

    float3 TangentX = { 1 + Sign * a * Square(TangentY.x), -Sign * TangentY.x, Sign * b };
    float3 TangentZ = { b, -TangentY.z, Sign + a * Square(TangentY.z) };
    
    //return float3x3(TangentX, TangentY, TangentZ);
    return float3x3(TangentX, TangentY, TangentZ);
}

uint3 Rand3DPCG16(int3 p)
{
    uint3 v = uint3(p);
    v = v * 1664525u + 1013904223u;

    v.x += v.y * v.z;
    v.y += v.z * v.x;
    v.z += v.x * v.y;
    v.x += v.y * v.z;
    v.y += v.z * v.x;
    v.z += v.x * v.y;

    return v >> 16u;
}

uint ReverseBits32(uint bits)
{
    bits = (bits << 16) | (bits >> 16);
    bits = ((bits & 0x00ff00ff) << 8) | ((bits & 0xff00ff00) >> 8);
    bits = ((bits & 0x0f0f0f0f) << 4) | ((bits & 0xf0f0f0f0) >> 4);
    bits = ((bits & 0x33333333) << 2) | ((bits & 0xcccccccc) >> 2);
    bits = ((bits & 0x55555555) << 1) | ((bits & 0xaaaaaaaa) >> 1);
    return bits;
}

float2 Hammersley16(uint Index, uint NumSamples, uint2 Random)
{
    float E1 = frac((float) Index / NumSamples + float(Random.x) * (1.0 / 65536.0));
    float E2 = float((ReverseBits32(Index) >> 16) ^ Random.y) * (1.0 / 65536.0);
    return float2(E1, E2);
}

//float VisibleGGXPDF(float3 V, float3 H, float a2)
//{
//    float NoV = V.z;
//    float NoH = H.z;
//    float VoH = dot(V, H);
//
//    float d = (NoH * a2 - NoH) * NoH + 1;
//    float D = a2 / (PI * d * d);
//
//    float PDF = 2 * VoH * D / (NoV + sqrt(NoV * (NoV - NoV * a2) + a2));
//    return PDF;
//}

float VisibleGGXPDF(float3 V, float3 H, float a2)
{
    float NoV = V.y;
    float NoH = H.y;
    float VoH = dot(V, H);

    float d = (NoH * a2 - NoH) * NoH + 1;
    float D = a2 / (PI * d * d);

    float PDF = 2 * VoH * D / (NoV + sqrt(NoV * (NoV - NoV * a2) + a2));
    return PDF;
}

//float4 ImportanceSampleVisibleGGX(float2 DiskE, float a2, float3 V)
//{
//    float a = sqrt(a2);
//
//    float3 Vh = normalize(float3(a * V.xy, V.z));
//
//    float LenSq = Vh.x * Vh.x + Vh.y * Vh.y;
//    float3 Tangent0 = LenSq > 0 ? float3(-Vh.y, Vh.x, 0) * rsqrt(LenSq) : float3(1, 0, 0);
//    float3 Tangent1 = cross(Vh, Tangent0);
//
//    float2 p = DiskE;
//    float s = 0.5 + 0.5 * Vh.z;
//    p.y = (1 - s) * sqrt(1 - p.x * p.x) + s * p.y;
//
//    float3 H;
//    H = p.x * Tangent0;
//    H += p.y * Tangent1;
//    H += sqrt(saturate(1 - dot(p, p))) * Vh;
//
//    H = normalize(float3(a * H.xy, max(0.0, H.z)));
//
//    return float4(H, VisibleGGXPDF(V, H, a2));
//}

float4 ImportanceSampleVisibleGGX(float2 DiskE, float a2, float3 V)
{
    float a = sqrt(a2);

    float3 Vh = normalize(float3(a * V.x, V.y, a * V.z));

    float LenSq = Vh.x * Vh.x + Vh.z * Vh.z;
    float3 Tangent0 = LenSq > 0 ? float3(-Vh.z, Vh.x, 0) * rsqrt(LenSq) : float3(1, 0, 0);
    float3 Tangent1 = cross(Vh, Tangent0);

    float2 p = DiskE;
    float s = 0.5 + 0.5 * Vh.y;
    p.y = (1 - s) * sqrt(1 - p.x * p.x) + s * p.y;

    float3 H;
    H = p.x * Tangent0;
    H += p.y * Tangent1;
    H += sqrt(saturate(1 - dot(p, p))) * Vh;

    //H = normalize(float3(a * H.xz, max(0.0, H.y)));
    H = normalize(float3(a * H.x, max(0.0, H.y), a * H.z));

    //return float4(H, VisibleGGXPDF(V, H, a2));
    return float4(H, VisibleGGXPDF(V, H, a2));
}

float2 UniformSampleDisk(float2 E)
{
    float Theta = 2 * PI * E.x;
    float Radius = sqrt(E.y);
    return Radius * float2(cos(Theta), sin(Theta));
}

struct FSSRTRay
{
    float3 RayStartScreen;
    float3 RayStepScreen;

    float CompareTolerance;
};

float GetStepScreenFactorToClipAtScreenEdge(float2 RayStartScreen, float2 RayStepScreen)
{
    const float RayStepScreenInvFactor = 0.5 * length(RayStepScreen);
    const float2 S = 1 - max(abs(RayStepScreen + RayStartScreen * RayStepScreenInvFactor) - RayStepScreenInvFactor, 0.0f) / abs(RayStepScreen);

    const float RayStepFactor = min(S.x, S.y) / RayStepScreenInvFactor;

    return RayStepFactor;
}

void ReprojectHit(float3 HitUVz, out float2 OutPrevUV, matrix ClipToPrevClip)
{
    float2 ScreenPos = ViewportUVToScreenPos(HitUVz.xy);
    float4 ThisClip = float4(ScreenPos, HitUVz.z, 1);
    float4 PrevClip = mul(ClipToPrevClip, ThisClip);
    float2 PrevScreen = PrevClip.xy / PrevClip.w;
    
    OutPrevUV = ScreenPosToViewportUV(PrevScreen);
}

FSSRTRay InitScreenSpaceRayFromWorldSpace(float3 RayOriginTranslatedWorld, float3 WorldRayDirection, float SceneDepth, ViewBuffer View)
{
    float4 RayStartClip = mul(View.WorldToProjection, float4(RayOriginTranslatedWorld, 1));
    float4 RayEndClip = mul(View.WorldToProjection, float4(RayOriginTranslatedWorld + WorldRayDirection * SceneDepth, 1));

    float3 RayStartScreen = RayStartClip.xyz * rcp(RayStartClip.w);
    float3 RayEndScreen = RayEndClip.xyz * rcp(RayEndClip.w);

    float4 RayDepthClip = RayStartClip + mul(View.ViewToProjection, float4(0, 0, SceneDepth, 0));
    float3 RayDepthScreen = RayDepthClip.xyz * rcp(RayDepthClip.w);

    FSSRTRay Ray;
    Ray.RayStartScreen = RayStartScreen;
    Ray.RayStepScreen = RayEndScreen - RayStartScreen;
	
    Ray.RayStepScreen *= GetStepScreenFactorToClipAtScreenEdge(RayStartScreen.xy, Ray.RayStepScreen.xy);
    Ray.CompareTolerance = max(abs(Ray.RayStepScreen.z), (RayStartScreen.z - RayDepthScreen.z) * 4);

    return Ray;
}

bool CastScreenSpaceRay(
	Texture2D Texture, SamplerState Sampler,
	FSSRTRay Ray,
	float Roughness,
	uint NumSteps, float StepOffset,
	float4 HZBUvFactorAndInvFactor,
    ViewBuffer View,
	out float3 OutHitUVz,
	out float Level)
{
    const float3 RayStartScreen = Ray.RayStartScreen;
    float3 RayStepScreen = Ray.RayStepScreen;

    float3 RayStartUVz = float3((RayStartScreen.xy * float2(0.5, -0.5) + 0.5) * HZBUvFactorAndInvFactor.xy, RayStartScreen.z);
    float3 RayStepUVz = float3(RayStepScreen.xy * float2(0.5, -0.5) * HZBUvFactorAndInvFactor.xy, RayStepScreen.z);
	
    const float Step = 1.0 / NumSteps;
    float CompareTolerance = Ray.CompareTolerance * Step;
	
    float LastDiff = 0;
    Level = 1;

    RayStepUVz *= Step;
    float3 RayUVz = RayStartUVz + RayStepUVz * StepOffset;
	
    float4 MultipleSampleDepthDiff;
    bool4 bMultipleSampleHit;
    bool bFoundAnyHit = false;
	
    uint i;

    for (i = 0; i < NumSteps; i += SSRT_SAMPLE_BATCH_SIZE)
    {
        float2 SamplesUV[SSRT_SAMPLE_BATCH_SIZE];
        float4 SamplesZ;
        float4 SamplesMip;

        [unroll(SSRT_SAMPLE_BATCH_SIZE)]
        for (uint j = 0; j < SSRT_SAMPLE_BATCH_SIZE; j++)
        {
            SamplesUV[j] = RayUVz.xy + (float(i) + float(j + 1)) * RayStepUVz.xy;
            SamplesZ[j] = RayUVz.z + (float(i) + float(j + 1)) * RayStepUVz.z;
        }
		
        SamplesMip.xy = Level;
        Level += (8.0 / NumSteps) * Roughness;
		
        SamplesMip.zw = Level;
        Level += (8.0 / NumSteps) * Roughness;

        float4 SampleDepth;
        {
            [unroll(SSRT_SAMPLE_BATCH_SIZE)]
            for (uint j = 0; j < SSRT_SAMPLE_BATCH_SIZE; j++)
            {
                SampleDepth[j] = Texture.SampleLevel(Sampler, SamplesUV[j], SamplesMip[j]).r;
            }
        }

        MultipleSampleDepthDiff = SamplesZ - SampleDepth;
        bMultipleSampleHit = abs(MultipleSampleDepthDiff + CompareTolerance) < CompareTolerance;
        bFoundAnyHit = any(bMultipleSampleHit);

        [branch]
        if (bFoundAnyHit)
        {
            break;
        }

        LastDiff = MultipleSampleDepthDiff.w;
    }
    [branch]
    if (bFoundAnyHit)
    {
        float DepthDiff0 = MultipleSampleDepthDiff[2];
        float DepthDiff1 = MultipleSampleDepthDiff[3];
        float Time0 = 3;

        [flatten]
        if (bMultipleSampleHit[2])
        {
            DepthDiff0 = MultipleSampleDepthDiff[1];
            DepthDiff1 = MultipleSampleDepthDiff[2];
            Time0 = 2;
        }
        
        [flatten]
        if (bMultipleSampleHit[1])
        {
            DepthDiff0 = MultipleSampleDepthDiff[0];
            DepthDiff1 = MultipleSampleDepthDiff[1];
            Time0 = 1;
        }
        
        [flatten]
        if (bMultipleSampleHit[0])
        {
            DepthDiff0 = LastDiff;
            DepthDiff1 = MultipleSampleDepthDiff[0];
            Time0 = 0;
        }

        Time0 += float(i);

        float Time1 = Time0 + 1;

        float TimeLerp = saturate(DepthDiff0 / (DepthDiff0 - DepthDiff1));
        float IntersectTime = Time0 + TimeLerp;
				
        OutHitUVz = RayUVz + RayStepUVz * IntersectTime;

        OutHitUVz.xy *= HZBUvFactorAndInvFactor.zw;
        OutHitUVz.xy = OutHitUVz.xy * float2(2, -2) + float2(-1, 1);
        //OutHitUVz.xy = OutHitUVz.xy * View.ScreenPositionScaleBias.xy + View.ScreenPositionScaleBias.wz;
        OutHitUVz.xy = OutHitUVz.xy * float2(0.5f, -0.5f) + float2(0.5f, 0.5f);
    }
    else
    {
        OutHitUVz = float3(0, 0, 0);
    }
	
    return bFoundAnyHit;
}

bool RayCast(
	Texture2D Texture, SamplerState Sampler,
	float3 RayOriginTranslatedWorld, float3 RayDirection,
	float Roughness, float SceneDepth,
	uint NumSteps, float StepOffset,
	float4 HZBUvFactorAndInvFactor,
    ViewBuffer View,
	out float3 OutHitUVz,
	out float Level)
{
    FSSRTRay Ray = InitScreenSpaceRayFromWorldSpace(RayOriginTranslatedWorld, RayDirection, SceneDepth, View);
    
    return CastScreenSpaceRay( Texture, Sampler, Ray, Roughness, NumSteps, StepOffset, HZBUvFactorAndInvFactor, View, OutHitUVz, Level);
}

float ComputeRayHitSqrDistance(float3 OriginTranslatedWorld, float3 HitUVz, ViewBuffer View)
{
    //float2 HitScreenPos = (HitUVz.xy - View.ScreenPositionScaleBias.wz) / View.ScreenPositionScaleBias.xy;
    float2 HitScreenPos = (HitUVz.xy - float2(0.5f, 0.5f)) / float2(0.5f, -0.5f);
    float HitSceneDepth = ConvertFromDeviceZ(HitUVz.z, View.InvDeviceZToWorldZTransform);

    //float3 HitTranslatedWorld = mul(float4(HitScreenPos * HitSceneDepth, HitSceneDepth, 1), View.ScreenToTranslatedWorld).xyz;
    float3 HitTranslatedWorld = mul(View.ScreenToTranslatedWorld, float4(HitScreenPos * HitSceneDepth, HitSceneDepth, 1)).xyz;

    float3 V = OriginTranslatedWorld - HitTranslatedWorld;
    return dot(V, V);
}

float3 Luminance(float3 LinearColor)
{
    return dot(LinearColor, float3(0.3, 0.59, 0.11));
}

float4 Main_PS(PixelShaderInput IN) : SV_Target
{
    ConstantBuffer<ViewBuffer> View = ResourceDescriptorHeap[BindlessResources.ViewBufferIndex];
    ConstantBuffer<SSRData> SSRBuffer = ResourceDescriptorHeap[BindlessResources.SSRBufferIndex];
    ConstantBuffer<StaticSamplers> StaticSamplersBuffer = ResourceDescriptorHeap[BindlessResources.StaticSamplerBufferIndex];
    
    Texture2D DeferredImage = ResourceDescriptorHeap[SSRBuffer.DeferredColorTexture];
    Texture2D ColorImage = ResourceDescriptorHeap[SSRBuffer.BaseColorTexture];
    Texture2D NormalImage = ResourceDescriptorHeap[SSRBuffer.WorldNormalTexture];
    Texture2D MaskImage = ResourceDescriptorHeap[SSRBuffer.MasksTexture];
    Texture2D DepthImage = ResourceDescriptorHeap[SSRBuffer.DepthTexture];
    Texture2D HzbImage = ResourceDescriptorHeap[SSRBuffer.HzbTexture];
    
    SamplerState PointSampler = ResourceDescriptorHeap[StaticSamplersBuffer.PointSamplerIndex];
    SamplerState PointClampSampler = ResourceDescriptorHeap[StaticSamplersBuffer.PointClampIndex];
    SamplerState LinearSampler = ResourceDescriptorHeap[StaticSamplersBuffer.LinearSamplerIndex];
    
    float2 UV = IN.UVAndScreenPos.xy;
    float2 ScreenPos = IN.UVAndScreenPos.zw;
    uint2 PixelPos = (uint2) IN.Position.xy;
    
    float3 DeferredColor = DeferredImage.Sample(LinearSampler, UV).xyz;
    float2 EncodedNormal = NormalImage.Sample(LinearSampler, UV).xy;
    float3 WorldNormal = DecodeNormal(EncodedNormal);
    float Depth = DepthImage.Sample(LinearSampler, UV).x;
    float SceneDepth = ConvertFromDeviceZ(Depth, View.InvDeviceZToWorldZTransform);
    float3 PositionTranslatedWorld = mul(View.ScreenToTranslatedWorld, float4(ScreenPos * SceneDepth, SceneDepth, 1)).xyz;
    float3 V = normalize(View.CameraPos - PositionTranslatedWorld);
    
    float4 Masks = MaskImage.Sample(LinearSampler, UV);
    float Roughness = Masks.g;
    float RoughnessFade = GetRoughnessFade(Roughness, SSRBuffer.RoughnessFade);
    
    [branch]
    if ((uint) (Masks.a * 255) == 0)
        return 0;
    
    [branch]
    if (RoughnessFade <= 0.0f)
        return 0;
    
    float a = Roughness * Roughness;
    float a2 = a * a;
    
    float ClosestHitDistanceSqr = 100000.0f;
    
    // TODO: UE setup didnt work
    //float4 HZBUvFactorAndInvFactor = float4(1272.0f / (512.0f * 2.0f), 607.0f / (256.0f * 2.0f), 0, 0);
    float4 HZBUvFactorAndInvFactor = float4(1.0f, 1.0f, 0, 0);
    HZBUvFactorAndInvFactor.zw = float2(1.0f, 1.0f) / HZBUvFactorAndInvFactor.xy;
        
#define SSR_QUALITY 3
    
#if SSR_QUALITY == 1
	uint NumSteps = 8;
	uint NumRays = 1;
	bool bGlossy = false;
#elif SSR_QUALITY == 2
	uint NumSteps = 16;
	uint NumRays = 1;
	bool bGlossy = false;
#elif SSR_QUALITY == 3
	uint NumSteps = 8;
	uint NumRays = 4;
	bool bGlossy = true;
#else // SSR_QUALITY == 4
    uint NumSteps = 12;
    uint NumRays = 12;
    bool bGlossy = true;
#endif

    float4 Result = 0;
    
    if(NumRays > 1)
    {
        float2 Noise;
        Noise.x = InterleavedGradientNoise(IN.Position.xy, View.FrameIndexMod8);
        Noise.y = InterleavedGradientNoise(IN.Position.xy, View.FrameIndexMod8 * 117);
        //float2 Noise = 0;
        
        uint2 Random = Rand3DPCG16(int3(PixelPos, View.FrameIndexMod8)).xy;
        //uint2 Random = 0;
        
        float3x3 TangentBasis = GetTangentBasis(WorldNormal);
        float3 TangentV = mul(V, TangentBasis);
        
        float Count = 0;
        if(Roughness < 0.1f)
        {
            NumSteps = min(NumSteps * NumRays, 24);
            NumRays = 1;
        }
        
        for (uint i = 0; i < NumRays; i++)
        {
            float StepOffset = Noise.x - 0.5f;

            float2 E = Hammersley16(i, NumRays, Random);
            //float2 E = 0;
            float3 H = mul(TangentBasis, ImportanceSampleVisibleGGX(UniformSampleDisk(E), a2, TangentV).xyz);
            float3 L = 2 * dot(V, H) * H - V;
            
            float3 HitUVz;
            float Level;
            
            if( Roughness < 0.1f )
            {
                L = reflect(-V, WorldNormal);
            }
            
            bool bHit = RayCast(HzbImage, PointClampSampler, PositionTranslatedWorld, L, Roughness, SceneDepth, NumSteps, StepOffset, HZBUvFactorAndInvFactor, View, HitUVz, Level);
            //bool bHit = RayCast(DepthImage, PointClampSampler, PositionTranslatedWorld, L, Roughness, SceneDepth, NumSteps, StepOffset, HZBUvFactorAndInvFactor, View, HitUVz, Level);
            //return float4(HitUVz.xyz, 1);
            
            [branch]
            if(bHit)
            {
                //ClosestHitDistanceSqr = min(ClosestHitDistanceSqr, ComputeRayHitSqrDistance(PositionTranslatedWorld, HitUVz, View));

                float2 SampleUV = 0;
                ReprojectHit(HitUVz, SampleUV, View.ClipToPreviousClip);
                
                float4 SampleColor = DeferredImage.Sample(LinearSampler, SampleUV);
                SampleColor.a = 1;
                SampleColor.rgb *= rcp(1 + Luminance(SampleColor.rgb));
                Result += SampleColor;
            }
        }
        
        Result /= max(NumRays, 0.0001);
        Result.rgb *= rcp(1 - Luminance(Result.rgb));
    }
    else
    {
        float StepOffset = InterleavedGradientNoise(IN.Position.xy, View.FrameIndexMod8);
		StepOffset -= 0.5;
		
        float3 L = reflect(-V, WorldNormal);
		
		float3 HitUVz;
		float Level = 0;
        bool bHit = RayCast(HzbImage, PointClampSampler, PositionTranslatedWorld, L, Roughness, SceneDepth, NumSteps, StepOffset, HZBUvFactorAndInvFactor, View, HitUVz, Level);

        [branch]
		if( bHit )
		{
            //ClosestHitDistanceSqr = ComputeRayHitSqrDistance(PositionTranslatedWorld, HitUVz, View);

			float2 SampleUV = 0;
			ReprojectHit(HitUVz, SampleUV, View.ClipToPreviousClip);

            float4 SampleColor = DeferredImage.Sample(LinearSampler, SampleUV);
            Result = SampleColor;
            Result.a = 1;
        }
    }
    
    
    Result *= RoughnessFade;
    Result *= SSRBuffer.Intensity;
    
    // TODO: mask sky by material id
    return SceneDepth > 9000.0f ? 0 : Result;
}