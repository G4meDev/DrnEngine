
#define SAMPLESET_ARRAY_SIZE 6
#define SAMPLE_STEPS 3

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
    uint AoSetupIndex;
    uint StaticSamplerBufferIndex;
    uint HzbIndex;
    
    uint RandomTextureIndex;
    float2 ViewportUVToRandomUV;
    float Pad;

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
};


float ConvertFromDeviceZ(float DeviceZ, float4 InvDeviceZToWorldZTransform)
{
    return DeviceZ * InvDeviceZToWorldZTransform[0] + InvDeviceZToWorldZTransform[1] + 1.0f / (DeviceZ * InvDeviceZToWorldZTransform[2] - InvDeviceZToWorldZTransform[3]);
}

float GetHZBDepth(float2 ScreenPos, float MipLevel, Texture2D Texture, SamplerState State, float4 InvDeviceZToWorldZTransform)
{
    //const float2 HZBRemapping_Scale = float2(636, -303) / float2(512, 256) / 2;
    const float2 HZBRemapping_Scale = float2(1.0f, -1.0f) / 2;
    //const float2 HZBRemapping_Scale = float2(512, 256) / float2(636, -303) / 2;
    const float2 HZBRemapping_Bias = float2(0.5, 0.5);
    
    float2 HZBUV = HZBRemapping_Scale * ScreenPos + HZBRemapping_Bias;
    float HZBDepth = Texture.SampleLevel(State, HZBUV, MipLevel).r;
    return ConvertFromDeviceZ(HZBDepth, InvDeviceZToWorldZTransform);
}

float ComputeMipLevel(int sampleid, int step)
{
    float SamplePos = (sampleid + 0.5f) / SAMPLESET_ARRAY_SIZE;
    float HzbStepMipLevelFactorValue = 1;
    float Scale = (step + 1) / (float) SAMPLE_STEPS;

    //return 0;
    return log2(HzbStepMipLevelFactorValue * Scale * SamplePos);
}

float3 ReconstructCSPos(float SceneDepth, float2 ScreenPos)
{
    return float3(ScreenPos * SceneDepth, SceneDepth);
}

#define OPTIMIZATION_O1 1

float3 WedgeWithNormal(float2 ScreenSpacePosCenter, float2 InLocalRandom, float3 InvFovFix, float3 ViewSpacePosition, float3 ScaledViewSpaceNormal, float InvHaloSize, float MipLevel, Texture2D HzbTexture, SamplerState State, float4 InvDeviceZToWorldZTransform)
{
    float2 ScreenSpacePosL = ScreenSpacePosCenter + InLocalRandom;
    float2 ScreenSpacePosR = ScreenSpacePosCenter - InLocalRandom;

    //float AbsL = GetHZBDepth(ScreenSpacePosCenter, MipLevel, HzbTexture, State, InvDeviceZToWorldZTransform);
    float AbsL = GetHZBDepth(ScreenSpacePosL, MipLevel, HzbTexture, State, InvDeviceZToWorldZTransform);
    float AbsR = GetHZBDepth(ScreenSpacePosR, MipLevel, HzbTexture, State, InvDeviceZToWorldZTransform);
    
    float3 SamplePositionL = ReconstructCSPos(AbsL, ScreenSpacePosL);
    float3 SamplePositionR = ReconstructCSPos(AbsR, ScreenSpacePosR);

    float3 DeltaL = (SamplePositionL - ViewSpacePosition) * InvFovFix;
    float3 DeltaR = (SamplePositionR - ViewSpacePosition) * InvFovFix;

#if OPTIMIZATION_O1
	float InvNormAngleL = saturate(dot(DeltaL, ScaledViewSpaceNormal) / dot(DeltaL, DeltaL));
	float InvNormAngleR = saturate(dot(DeltaR, ScaledViewSpaceNormal) / dot(DeltaR, DeltaR));
	float Weight = 1;
#else
    float InvNormAngleL = saturate(dot(DeltaL, ScaledViewSpaceNormal) * rsqrt(dot(DeltaL, DeltaL)));
    float InvNormAngleR = saturate(dot(DeltaR, ScaledViewSpaceNormal) * rsqrt(dot(DeltaR, DeltaR)));

    float Weight =
		  saturate(1.0f - length(DeltaL) * InvHaloSize)
		* saturate(1.0f - length(DeltaR) * InvHaloSize);
#endif
    
    //return AbsL.xxx;
    return float3(InvNormAngleL, InvNormAngleR, Weight);
    
}

float Square(float x) { return x * x; }

float Main_PS(PixelShaderInput IN) : SV_Target
{
    ConstantBuffer<ViewBuffer> View = ResourceDescriptorHeap[BindlessResources.ViewBufferIndex];
    
    Texture2D SetupImage = ResourceDescriptorHeap[BindlessResources.AoSetupIndex];
    Texture2D HzbImage = ResourceDescriptorHeap[BindlessResources.HzbIndex];
    Texture2D RandomImage = ResourceDescriptorHeap[BindlessResources.RandomTextureIndex];
    ConstantBuffer<StaticSamplers> StaticSamplersBuffer = ResourceDescriptorHeap[BindlessResources.StaticSamplerBufferIndex];
    
    SamplerState PointSampler = ResourceDescriptorHeap[StaticSamplersBuffer.PointSamplerIndex];
    SamplerState PointClampSampler = ResourceDescriptorHeap[StaticSamplersBuffer.PointClampIndex];
    
    float2 UV = IN.UVAndScreenPos.xy;
    float2 ScreenPos = IN.UVAndScreenPos.zw;
    
    float Ratio = (float) View.RenderSize.x / (float) View.RenderSize.y;
    float InvTanHalfFov = View.InvTanHalfFov;
    float3 FovFix = float3(InvTanHalfFov, Ratio * InvTanHalfFov, 1);
    float3 InvFovFix = 1.0f / FovFix;
    
    float AORadiusInShader = 0.1;
    float AmbientOcclusionBias = 0.0003;
    float ScaleFactor = 4;
     
    float4 SetupSample = SetupImage.Sample(PointClampSampler, UV);
    
    //float SceneDepth = SetupSample.a * Constant_Float16F_Scale;
    float SceneDepth = ConvertFromDeviceZ(SetupSample.a, View.InvDeviceZToWorldZTransform);
    float3 ViewSpacePosition = ReconstructCSPos(SceneDepth, ScreenPos);

    float3 WorldNormal = SetupSample.xyz * 2 - 1;
    float3 ViewSpaceNormal = normalize(mul((float3x3) View.WorldToView, WorldNormal));
    
    float ActualAORadius = AORadiusInShader * SceneDepth;
    
    float2 RandomVec = (RandomImage.Sample(PointSampler, UV * BindlessResources.ViewportUVToRandomUV).rg * 2 - 1) * ActualAORadius;
    
    ViewSpacePosition += AmbientOcclusionBias * SceneDepth * ScaleFactor * (ViewSpaceNormal * FovFix);
    
    float2 FovFixXY = FovFix.xy * (1.0f / ViewSpacePosition.z);
    float4 RandomBase = float4(RandomVec, -RandomVec.y, RandomVec.x) * float4(FovFixXY, FovFixXY);
    float2 ScreenSpacePos = ViewSpacePosition.xy / ViewSpacePosition.z;
    
    float InvHaloSize = 1.0f / (ActualAORadius * FovFixXY.x * 2);
    
    
    float3 ScaledViewSpaceNormal = ViewSpaceNormal;
#if OPTIMIZATION_O1
    ScaledViewSpaceNormal *= 0.08f * 10;
#endif
    
    float2 WeightAccumulator = 0.0001f;
    [unroll]
    for (int i = 0; i < SAMPLESET_ARRAY_SIZE; ++i)
    {
        float2 UnrotatedRandom = OcclusionSamplesOffsets[i].xy;
        float2 LocalRandom = (UnrotatedRandom.x * RandomBase.xy + UnrotatedRandom.y * RandomBase.zw);
        
        float3 LocalAccumulator = 0;

        [unroll]
        for (uint step = 0; step < SAMPLE_STEPS; ++step)
        {
            float Scale = (step + 1) / (float) SAMPLE_STEPS;
            float MipLevel = ComputeMipLevel(i, step);

            float3 StepSample = WedgeWithNormal(ScreenSpacePos, Scale * LocalRandom, InvFovFix, ViewSpacePosition, ScaledViewSpaceNormal, InvHaloSize, MipLevel, HzbImage, PointClampSampler, View.InvDeviceZToWorldZTransform);

            LocalAccumulator = lerp(LocalAccumulator, float3(max(LocalAccumulator.xy, StepSample.xy), 1), StepSample.z);

            //return StepSample.x / 50;
        }

        WeightAccumulator += float2(Square(1 - LocalAccumulator.x) * LocalAccumulator.z, LocalAccumulator.z);
        WeightAccumulator += float2(Square(1 - LocalAccumulator.y) * LocalAccumulator.z, LocalAccumulator.z);
    }
    
    return WeightAccumulator.x / WeightAccumulator.y;

}