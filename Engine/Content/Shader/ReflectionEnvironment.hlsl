
#define PI 3.1415926535897932f

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

struct SSRData
{
    uint BaseColorTexture;
    uint WorldNormalTexture;
    uint MasksTexture;
    uint DepthTexture;
    uint SSRTexture;
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


float ConvertFromDeviceZ(float DeviceZ, float4 InvDeviceZToWorldZTransform)
{
    return DeviceZ * InvDeviceZToWorldZTransform[0] + InvDeviceZToWorldZTransform[1] + 1.0f / (DeviceZ * InvDeviceZToWorldZTransform[2] - InvDeviceZToWorldZTransform[3]);
}

float Square(float x) { return x * x; }

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
    
    SamplerState PointSampler = ResourceDescriptorHeap[StaticSamplersBuffer.PointSamplerIndex];
    SamplerState PointClampSampler = ResourceDescriptorHeap[StaticSamplersBuffer.PointClampIndex];
    SamplerState LinearSampler = ResourceDescriptorHeap[StaticSamplersBuffer.LinearSamplerIndex];
    
    float2 UV = IN.UVAndScreenPos.xy;
    float2 ScreenPos = IN.UVAndScreenPos.zw;
    uint2 PixelPos = (uint2) IN.Position.xy;
    
    float4 SSRColor = SSRImage.Sample(LinearSampler, UV);
    float3 Normal = NormalImage.Sample(LinearSampler, UV).xyz;
    float3 WorldNormal = Normal * 2 - 1;
    float Depth = DepthImage.Sample(LinearSampler, UV).x;
    float SceneDepth = ConvertFromDeviceZ(Depth, View.InvDeviceZToWorldZTransform);
    float3 PositionTranslatedWorld = mul(View.ScreenToTranslatedWorld, float4(ScreenPos * SceneDepth, SceneDepth, 1)).xyz;
    float3 V = normalize(View.CameraPos - PositionTranslatedWorld);
    
    return SSRColor;
}