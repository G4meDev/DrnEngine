#include "Common.hlsl"

#define MAX_REFLECTION_CAPTURE_COUNT 32

struct Resources
{
    uint ViewBufferIndex;
    uint SSRBufferIndex;
    uint StaticSamplerBufferIndex;
    uint LightGridIndex;
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
    ConstantBuffer<LightGridData> LightGrid = ResourceDescriptorHeap[BindlessResources.LightGridIndex];
    ConstantBuffer<StaticSamplers> StaticSamplersBuffer = ResourceDescriptorHeap[BindlessResources.StaticSamplerBufferIndex];
    
    Texture2D ColorImage = ResourceDescriptorHeap[SSRBuffer.BaseColorTexture];
    Texture2D NormalImage = ResourceDescriptorHeap[SSRBuffer.WorldNormalTexture];
    Texture2D MaskImage = ResourceDescriptorHeap[SSRBuffer.MasksTexture];
    Texture2D DepthImage = ResourceDescriptorHeap[SSRBuffer.DepthTexture];
    Texture2D SSRImage = ResourceDescriptorHeap[SSRBuffer.SSRTexture];
    Texture2D AOImage = ResourceDescriptorHeap[SSRBuffer.AOTexture];
    
    SamplerState LinearClampSampler = ResourceDescriptorHeap[StaticSamplersBuffer.LinearClampIndex];
    SamplerState PointClampSampler = ResourceDescriptorHeap[StaticSamplersBuffer.PointClampIndex];
    
    float2 UV = IN.UVAndScreenPos.xy;
    float2 ScreenPos = IN.UVAndScreenPos.zw;
    
    float Depth = DepthImage.Sample(PointClampSampler, UV).x;
    float SceneDepth = ConvertFromDeviceZ(Depth, View.InvDeviceZToWorldZTransform);
    float3 WorldPosition = mul(View.ScreenToTranslatedWorld, float4(ScreenPos * SceneDepth, SceneDepth, 1)).xyz;

    float4 Masks = MaskImage.Sample(PointClampSampler, UV);
    float SSAO = AOImage.Sample(PointClampSampler, UV).x;
    float4 SSR = SSRImage.Sample(PointClampSampler, UV);
    
    GBufferData GBuffer;
    GBuffer.BaseColor = ColorImage.Sample(PointClampSampler, UV).xyz;
    GBuffer.WorldNormal = DecodeNormal(NormalImage.Sample(PointClampSampler, UV).xy);
    GBuffer.Matallic = Masks.r;
    GBuffer.Roughness = Masks.g;
    GBuffer.AmbientOcclusion = Masks.b * SSAO;
    GBuffer.ShadingModel = FloatToUint8(Masks.a);
    
    if (GBuffer.ShadingModel == SHADING_MODEL_UNLIT)
        return 0;

    float3 Env = GetEnvironemntReflection(View, LightGrid, GBuffer, SSR, WorldPosition, IN.Position.xy, SceneDepth, LinearClampSampler);
    return float4(Env, 1);
}