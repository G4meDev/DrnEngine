
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
    uint AOTexture;
    uint PreintegratedGF;
    uint SkyCubemapTexture;
    uint SkyLightMipCount;
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

float3 ComputeF0(float3 BaseColor, float Metallic)
{
    float3 F0 = float3(0.04, 0.04, 0.04);
    F0 = lerp(F0, BaseColor, Metallic);
    return F0;
}

float3 GetOffSpecularPeakReflectionDir(float3 Normal, float3 ReflectionVector, float Roughness)
{
    float a = Square(Roughness);
    return lerp(Normal, ReflectionVector, (1 - a) * (sqrt(1 - a) + a));
}

half GetSpecularOcclusion(float NoV, float RoughnessSq, float AO)
{
    return saturate(pow(NoV + AO, RoughnessSq) - 1 + AO);
}

half3 EnvBRDF(half3 SpecularColor, half Roughness, half NoV, Texture2D PreIntegratedGF, SamplerState State)
{
    return SpecularColor;
    
    float2 AB = PreIntegratedGF.SampleLevel( State, float2(NoV, Roughness), 0).rg;

    float3 GF = SpecularColor * AB.x + saturate(50.0 * SpecularColor.g) * AB.y;
    return GF;
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
    
    SamplerState PointSampler = ResourceDescriptorHeap[StaticSamplersBuffer.PointSamplerIndex];
    SamplerState PointClampSampler = ResourceDescriptorHeap[StaticSamplersBuffer.PointClampIndex];
    SamplerState LinearSampler = ResourceDescriptorHeap[StaticSamplersBuffer.LinearSamplerIndex];
    
    float2 UV = IN.UVAndScreenPos.xy;
    float2 ScreenPos = IN.UVAndScreenPos.zw;
    uint2 PixelPos = (uint2) IN.Position.xy;
    
    float3 BaseColor = ColorImage.Sample(LinearSampler, UV).xyz;
    float4 Masks = MaskImage.Sample(LinearSampler, UV);
    float Metallic = Masks.r;
    float Roughness = Masks.g;
    float AO = Masks.b;
    
    float3 Normal = NormalImage.Sample(LinearSampler, UV).xyz;
    float3 WorldNormal = Normal * 2 - 1;
    float Depth = DepthImage.Sample(LinearSampler, UV).x;
    float SceneDepth = ConvertFromDeviceZ(Depth, View.InvDeviceZToWorldZTransform);
    
    float SSAO = AOImage.Sample(LinearSampler, UV).x;
    float3 DiffueColor = BaseColor - BaseColor * Metallic;
    float3 SpecularColor = ComputeF0(BaseColor, Metallic);
    float3 BentNormal = WorldNormal;
    

    float4 Color = float4(0, 0, 0, 1);
    float3 WorldPosition = mul(View.ScreenToTranslatedWorld, float4(ScreenPos * SceneDepth, SceneDepth, 1)).xyz;
        
    float3 CameraToPixel = normalize(WorldPosition - View.CameraPos);
    float3 ReflectionVector = reflect(CameraToPixel, WorldNormal);
        
    float3 N = WorldNormal;
    float3 V = -CameraToPixel;
        
    float3 R = 2 * dot(V, N) * N - V;
    float NoV = saturate(dot(N, V));
        
    R = GetOffSpecularPeakReflectionDir(N, R, Roughness);
        
    float4 SSR = SSRImage.Sample(LinearSampler, UV);
    Color.rgb = SSR.rgb;
    
    float CombinedAO = AO * SSAO;
    float RoughnessSq = Square(Roughness);
    float SpecularOcclusion = GetSpecularOcclusion(NoV, RoughnessSq, CombinedAO);
        
    float3 Iraddiance = 0;
        
    [branch]
    if(SSRBuffer.SkyCubemapTexture != 0)
    {
        const float RoughestMip = 1;
        const float MipScale = 1.2;
        
        half Level = RoughestMip - MipScale * log2(max(Roughness, 0.001));
        half MipLevel = SSRBuffer.SkyLightMipCount - 1 - Level;
        
        Iraddiance = SkyCubemapImage.SampleLevel(LinearSampler, ReflectionVector, MipLevel).xyz * (1 - SSR.a) * SpecularOcclusion;
    }

    Color.rgb += Iraddiance;
    Color.rgb *= EnvBRDF(SpecularColor, Roughness, NoV, PreintegeratedGFImage, LinearSampler);
    
    return Color;
}