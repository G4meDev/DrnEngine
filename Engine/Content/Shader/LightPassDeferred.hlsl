//#include "../Materials/Common.hlsl"

// TODO: move to common.hlsl

static const float PI = 3.14159265359;

float2 VSPosToScreenUV(float4 VSPos)
{
    float2 UV = VSPos.xy / VSPos.w;
    UV = UV / 2 + 0.5f;
    UV.y = 1 - UV.y;
    
    return UV;
}

float3 DecodeNormal(float3 Normal)
{
    return Normal * 2 - 1;
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

// --------------------------------------------------------------------------

struct Resources
{
    uint ViewBufferIndex;
    uint LightBufferIndex;
    uint StaticSamplerBufferIndex;
    uint BaseColorIndex;
    uint WorldNormalIndex;
    uint MasksIndex;
    uint DepthIndex;
    uint ShadowDepthIndex;
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
};

struct LightBuffer
{
    matrix LocalToProjection;
    float3 CameraPosition;
    float Pad_1;
    float3 LightPosition;
    float Radius;
    float3 Color;
    uint ShadowmapIndex;
};

struct StaticSamplers
{
    uint LinearSamplerIndex;
    uint PointSamplerIndex;
    uint LinearCompLessSamplerIndex;
};

struct ShadowDepth
{
    matrix WorldToProjectionMatrices[6];
    float3 LightPos;
    float NearZ;
    float Radius;
    float DepthBias;
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

VertexShaderOutput Main_VS(VertexInputPosUV IN)
{
    VertexShaderOutput OUT;

    ConstantBuffer<LightBuffer> Light = ResourceDescriptorHeap[BindlessResources.LightBufferIndex];
    
    OUT.Position = mul(Light.LocalToProjection, float4(IN.Position, 1.0f));
    OUT.UV = VSPosToScreenUV(OUT.Position);
    OUT.ScreenPos = OUT.Position.xy / OUT.Position.w;

    return OUT;
}

struct PixelShaderInput
{
    noperspective float2 UV : TEXCOORD;
    noperspective float2 ScreenPos : TEXCOORD1;
};

float4 Main_PS(PixelShaderInput IN) : SV_Target
{
    ConstantBuffer<ViewBuffer> View = ResourceDescriptorHeap[BindlessResources.ViewBufferIndex];
    ConstantBuffer<LightBuffer> Light = ResourceDescriptorHeap[BindlessResources.LightBufferIndex];
    
    ConstantBuffer<StaticSamplers> StaticSamplers = ResourceDescriptorHeap[BindlessResources.StaticSamplerBufferIndex];
    SamplerState LinearSampler = ResourceDescriptorHeap[StaticSamplers.LinearSamplerIndex];
    
    Texture2D BaseColorTexture = ResourceDescriptorHeap[BindlessResources.BaseColorIndex];
    Texture2D WorldNormalTexture = ResourceDescriptorHeap[BindlessResources.WorldNormalIndex];
    Texture2D MasksTexture = ResourceDescriptorHeap[BindlessResources.MasksIndex];
    Texture2D DepthTexture = ResourceDescriptorHeap[BindlessResources.DepthIndex];
    
    float4 BaseColor = BaseColorTexture.Sample(LinearSampler, IN.UV);
    float3 WorldNormal = WorldNormalTexture.Sample(LinearSampler, IN.UV).xyz;
    float4 Masks = MasksTexture.Sample(LinearSampler, IN.UV);
    float Depth = DepthTexture.Sample(LinearSampler, IN.UV).x;

    // TODO: reconstruct pos from camerapos + pixeldir * depth
    float4 WorldPos = mul(View.ProjectionToWorld, float4(IN.ScreenPos, Depth, 1));
    WorldPos.xyz /= WorldPos.w;
    
    float3 N = DecodeNormal(WorldNormal);
    
    float3 ToLight = Light.LightPosition - WorldPos.xyz;
    float DistanceSquare = dot(ToLight, ToLight);
    
    float3 L = ToLight * rsqrt(DistanceSquare);
    float NoL = saturate(dot( N, L ));
    
    float3 V = Light.CameraPosition - WorldPos.xyz;
    float3 H = normalize(normalize(V) + normalize(ToLight));

    float Distance = distance(WorldPos.xyz, Light.LightPosition);

// -------------------------------------------------------------------------
    float Distance2 = Distance * Distance;
    float DistanceAttenuation = 1 / (Distance2 + 1);

    float InvRadius = 1 / Light.Radius;

    float LightRadiusMask = Distance2 * InvRadius * InvRadius;

    LightRadiusMask *= LightRadiusMask;
    LightRadiusMask = 1 - LightRadiusMask;
    LightRadiusMask = clamp(LightRadiusMask, 0, 1);

    LightRadiusMask *= LightRadiusMask;

    float Attenuation = DistanceAttenuation * LightRadiusMask;
// -------------------------------------------------------------------------
    
    float3 F0 = float3(0.04, 0.04, 0.04);
    F0 = lerp(F0, BaseColor.rgb, Masks.r);
    
    float NDF = DistributionGGX(N, H, Masks.g);
    float G = GeometrySmith(N, V, L, Masks.g);
    float3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
    
    float3 kS = F;
    float3 kD = float3(1, 1, 1) - kS;
    kD *= 1.0 - Masks.r;
    
    float3 Specular = NDF * G * F / (max(dot(N, V), 0) * max(dot(N, L), 0) + 0.0001);
    
    float3 Result = (kD * BaseColor.xyz / PI + Specular) * NoL * Attenuation * Light.Color;

    float Shadow;
    
    if(Light.ShadowmapIndex != 0)
    {
        TextureCube<float> Shadowmap = ResourceDescriptorHeap[Light.ShadowmapIndex];

        float3 AbsLightVector = abs(ToLight);
        float MaxCoordinate = max(AbsLightVector.x, max(AbsLightVector.y, AbsLightVector.z));

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
        
        ConstantBuffer<ShadowDepth> ShadowDepthBuffer = ResourceDescriptorHeap[BindlessResources.ShadowDepthIndex];
        float4 ShadowPos = mul(ShadowDepthBuffer.WorldToProjectionMatrices[CubeFaceIndex], float4(WorldPos.xyz, 1));
        
        float ww = ShadowPos.z / ShadowPos.w;
        float ShadowDepthBias = -ShadowDepthBuffer.DepthBias / ShadowPos.w;
        
        SamplerComparisonState CompState = ResourceDescriptorHeap[StaticSamplers.LinearCompLessSamplerIndex];
        Shadow = Shadowmap.SampleCmp(CompState, -ToLight, ww + ShadowDepthBias);
    }
    
    return float4(Result * Shadow, 1);
}