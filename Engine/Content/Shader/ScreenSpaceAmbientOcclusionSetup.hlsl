
//const static float Constant_Float16F_Scale = 4096.0f * 32.0f;
const static float Constant_Float16F_Scale = 4096.0f;

struct Resources
{
    uint ViewBufferIndex;
    uint AoBufferIndex;
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
    float Pad_3;
		
    float3 CameraDir;
    float Pad_4;

    float4 InvDeviceZToWorldZTransform;

};

struct AoData
{
    uint DepthTexture;
    uint WorldNormalTexture;
    uint HzbTexture;
    uint SetupTexture;

    uint DownSampleTexture;
    uint RandomTexture;
    float2 ToRandomUV;

    float Intensity;
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
    float2 UV : TEXCOORD0;
    float4 Position : SV_Position;
};

VertexShaderOutput Main_VS(VertexInputPosUV IN)
{
    VertexShaderOutput OUT;

    ConstantBuffer<ViewBuffer> View = ResourceDescriptorHeap[BindlessResources.ViewBufferIndex];
    
    OUT.Position = mul(View.LocalToCameraView, float4(IN.Position, 1.0f));
    OUT.Position.z = 0;
    OUT.UV = IN.UV;

    return OUT;
}

struct PixelShaderInput
{
    float2 UV : TEXCOORD0;
};

float ConvertFromDeviceZ(float DeviceZ, float4 InvDeviceZToWorldZTransform)
{
    return DeviceZ * InvDeviceZToWorldZTransform[0] + InvDeviceZToWorldZTransform[1] + 1.0f / (DeviceZ * InvDeviceZToWorldZTransform[2] - InvDeviceZToWorldZTransform[3]);
}

// 0: not similar .. 1:very similar
float ComputeDepthSimilarity(float DepthA, float DepthB, float TweakScale)
{
    return saturate(1 - abs(DepthA - DepthB) * TweakScale);
}

float4 Main_PS(PixelShaderInput IN) : SV_Target
{
    ConstantBuffer<ViewBuffer> View = ResourceDescriptorHeap[BindlessResources.ViewBufferIndex];
    ConstantBuffer<AoData> AoBuffer = ResourceDescriptorHeap[BindlessResources.AoBufferIndex];
    ConstantBuffer<StaticSamplers> StaticSamplers = ResourceDescriptorHeap[BindlessResources.StaticSamplerBufferIndex];
    
    SamplerState LinearClampSampler = ResourceDescriptorHeap[StaticSamplers.LinearClampIndex];
    Texture2D DepthImage = ResourceDescriptorHeap[AoBuffer.DepthTexture];
    Texture2D NormalImage = ResourceDescriptorHeap[AoBuffer.WorldNormalTexture];
    
    float2 UV[4];
    UV[0] = IN.UV + float2(-0.5f, -0.5f) * View.InvSize;
    UV[1] = IN.UV + float2(0.5f, -0.5f) * View.InvSize;
    UV[2] = IN.UV + float2(-0.5f, 0.5f) * View.InvSize;
    UV[3] = IN.UV + float2(0.5f, 0.5f) * View.InvSize;
    
    float4 Samples[4];
    [unroll]
    for (uint i = 0; i < 4; ++i)
    {
        Samples[i].rgb = NormalImage.Sample(LinearClampSampler, UV[i]).xyz;
        Samples[i].a = DepthImage.Sample(LinearClampSampler, UV[i]).r;
    }
    
    float MaxDepth = max(max(Samples[0].a, Samples[1].a), max(Samples[2].a, Samples[3].a));
    float LinearDepth = ConvertFromDeviceZ(MaxDepth, View.InvDeviceZToWorldZTransform);
    
    float4 AvgColor = 0.0001f;
    
    const float ThresholdInverse = 0.0001f;
    [unroll]
    for (uint j = 0; j < 4; j++)
    {
        AvgColor += float4(Samples[j].rgb, 1) * ComputeDepthSimilarity(Samples[j].a, MaxDepth, ThresholdInverse);
    }
    AvgColor.rgb /= AvgColor.w;
    
    //return float4(AvgColor.xyz, LinearDepth / Constant_Float16F_Scale);
    return float4(AvgColor.xyz, MaxDepth);
}