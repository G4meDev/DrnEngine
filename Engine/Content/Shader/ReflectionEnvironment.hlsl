
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
    
    float3 SkyLightColor;
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
    
    float3 BaseColor = ColorImage.Sample(PointClampSampler, UV).xyz;
    float4 Masks = MaskImage.Sample(PointClampSampler, UV);
    float Metallic = Masks.r;
    float Roughness = Masks.g;
    float AO = Masks.b;
    
    uint ShadingModel = (uint)(Masks.a * 255.0f);
    if(ShadingModel != 1)
        return 0;
    
    float3 Normal = NormalImage.Sample(PointClampSampler, UV).xyz;
    float3 WorldNormal = Normal * 2 - 1;
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

    float3 Iraddiance = 0;
    float3 DiffuseTerm = 0;
        
    [branch]
    if(SSRBuffer.SkyCubemapTexture != 0)
    {
        float3 F = fresnelSchlickRoughness(max(dot(WorldNormal, -CameraToPixel), 0.0), SpecularColor, Roughness);
        const float RoughestMip = 1;
        const float MipScale = 1.2;
        
        half Level = RoughestMip - MipScale * log2(max(Roughness, 0.001));
        half MipLevel = SSRBuffer.SkyLightMipCount - 1 - Level;
        
        Iraddiance = SkyCubemapImage.SampleLevel(LinearSampler, ReflectionVector, MipLevel).xyz * SSRBuffer.SkyLightColor * (1 - SSR.a) * SpecularOcclusion;
        {
            float3 kS = F;
            float3 kD = 1 - kS;
            kD *= 1.0f - Metallic;
        
            float3 Sample = 0;
            // TODO: this is hack just make another cubemap at runtime for diffuse lookup
            Sample += SkyCubemapImage.SampleLevel(LinearSampler, WorldNormal, SSRBuffer.SkyLightMipCount - 1).xyz;
            Sample += SkyCubemapImage.SampleLevel(LinearSampler, WorldNormal, SSRBuffer.SkyLightMipCount - 2).xyz * 0.5f;
            Sample += SkyCubemapImage.SampleLevel(LinearSampler, WorldNormal, SSRBuffer.SkyLightMipCount - 3).xyz * 0.2f;
            DiffuseTerm = SSRBuffer.SkyLightColor * Sample * kD * DiffueColor;
        }
    }
    //float3 F = fresnelSchlickRoughness(max(dot(WorldNormal, -CameraToPixel), 0.0), SpecularColor, Roughness);

    SpecularTerm += Iraddiance;
    float3 BRDF = EnvBRDF(SpecularColor, Roughness, NoV, PreintegeratedGFImage, PointClampSampler);
    //float3 BRDF = EnvBRDFApprox(SpecularColor, Roughness, NoV);
    //float3 BRDF = EnvBRDFApproxNonmetal(Roughness, NoV);
    
    SpecularTerm *= BRDF;
    //return float4(SpecularTerm, 1);
    return float4(SpecularTerm + DiffuseTerm * CombinedAO, 1);
    //return float4( DiffuseTerm * CombinedAO, 1);
}