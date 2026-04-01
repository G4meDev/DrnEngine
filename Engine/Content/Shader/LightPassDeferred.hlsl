#include "Common.hlsl"

#define LIGHT_BITFLAG_POINTLIGHT 1
#define LIGHT_BITFLAG_SPOTLIGHT 2
#define LIGHT_BITFLAG_DIRECTIONAL 4
#define LIGHT_BITFLAG_SKY 8

#ifndef SPOTLIGHT_STENCIL_SIDES
#define SPOTLIGHT_STENCIL_SIDES 18
#endif

#ifndef SPOTLIGHT_STENCIL_SLICES
#define SPOTLIGHT_STENCIL_SLICES 12
#endif

// --------------------------------------------------------------------------

struct Resources
{
    uint ViewBufferIndex;
    uint LightDataIndex;
    uint StaticSamplerBufferIndex;
    uint BaseColorIndex;
    uint WorldNormalIndex;
    uint MasksIndex;
    uint DepthIndex;
    uint LightFlags;
    uint SSAOIndex;
    uint MasksBIndex;
};

ConstantBuffer<Resources> BindlessResources : register(b0);

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

VertexShaderOutput Main_VS(VertexInputPosUV IN, uint InVertexId : SV_VertexID)
{
    VertexShaderOutput OUT;
    ConstantBuffer<ViewBuffer> View = ResourceDescriptorHeap[BindlessResources.ViewBufferIndex];
    float3 WorldPosition;

    [branch]
    if (BindlessResources.LightFlags & LIGHT_BITFLAG_POINTLIGHT)
    {
        ConstantBuffer<PointLightData> LightBuffer = ResourceDescriptorHeap[BindlessResources.LightDataIndex];
        WorldPosition = IN.Position * LightBuffer.WorldPosAndScale.w + LightBuffer.WorldPosAndScale.xyz;
        OUT.Position = mul(View.WorldToProjection, float4(WorldPosition, 1.0f));
    }

    else if(BindlessResources.LightFlags & LIGHT_BITFLAG_SPOTLIGHT)
    {
        ConstantBuffer<SpotLightData> LightBuffer = ResourceDescriptorHeap[BindlessResources.LightDataIndex];
        
        float SphereRadius = LightBuffer.Attenution;
        float ConeAngle = LightBuffer.OuterRadius;

        uint NumSides = SPOTLIGHT_STENCIL_SIDES;
        uint NumSlices = SPOTLIGHT_STENCIL_SLICES;
        
        const float InvCosRadiansPerSide = 1.0f / cos(PI / (float) NumSides);
        const float ZRadius = SphereRadius * cos(ConeAngle);
        const float TanConeAngle = tan(ConeAngle);

        float3 LocalPosition = float3(1, 1, 1);
        
        uint CapIndexStart = NumSides * NumSlices;
        if (InVertexId < CapIndexStart)
        {
            uint SideIndex = InVertexId / NumSlices;
            uint SliceIndex = InVertexId % NumSlices;
            
            const float CurrentAngle = SideIndex * 2 * PI / (float) NumSides;
            const float DistanceDownConeDirection = ZRadius * SliceIndex / (float) (NumSlices - 1);
            const float SliceRadius = DistanceDownConeDirection * TanConeAngle * InvCosRadiansPerSide;
            LocalPosition = float3(SliceRadius * cos(CurrentAngle), SliceRadius * sin(CurrentAngle), ZRadius * SliceIndex / (float) (NumSlices - 1));
            //WorldPosition = mul(float4(LocalPosition, 1), StencilingConeTransform).xyz + StencilingPreViewTranslation;
        }
        else
        {
            const float CapRadius = ZRadius * tan(ConeAngle);

            uint VertexId = InVertexId - CapIndexStart;

            uint SideIndex = VertexId / NumSlices;
            uint SliceIndex = VertexId % NumSlices;
            
            const float UnadjustedSliceRadius = CapRadius * SliceIndex / (float) (NumSlices - 1);
            const float SliceRadius = UnadjustedSliceRadius * InvCosRadiansPerSide;
            const float ZDistance = sqrt(SphereRadius * SphereRadius - UnadjustedSliceRadius * UnadjustedSliceRadius);

            const float CurrentAngle = SideIndex * 2 * PI / (float) NumSides;
            LocalPosition = float3(SliceRadius * cos(CurrentAngle), SliceRadius * sin(CurrentAngle), ZDistance);
            //WorldPosition = mul(float4(LocalPosition, 1), StencilingConeTransform).xyz + StencilingPreViewTranslation;
        }
        
        WorldPosition = mul(LightBuffer.LocalToWorld, float4(LocalPosition, 1)).xyz;
        OUT.Position = mul(View.WorldToProjection, float4(WorldPosition, 1.0f));
    }

    else if (BindlessResources.LightFlags & (LIGHT_BITFLAG_DIRECTIONAL))
    {
        OUT.Position = mul(View.LocalToCameraView, float4(IN.Position, 1.0f));
        OUT.Position.z = 0;
    }
    
    else
    {
        WorldPosition = float3(1, 1, 1);
    }

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
    
    ConstantBuffer<StaticSamplers> StaticSamplers = ResourceDescriptorHeap[BindlessResources.StaticSamplerBufferIndex];
    SamplerState LinearSampler = ResourceDescriptorHeap[StaticSamplers.LinearSamplerIndex];
    
    Texture2D MasksTexture = ResourceDescriptorHeap[BindlessResources.MasksIndex];
    float4 Masks = MasksTexture.Sample(LinearSampler, IN.UV);
    
    uint ShadingModel = FloatToUint8(Masks.a);
    [branch]
    if (ShadingModel == SHADING_MODEL_UNLIT)
        return 0;
    
    Texture2D BaseColorTexture = ResourceDescriptorHeap[BindlessResources.BaseColorIndex];
    Texture2D WorldNormalTexture = ResourceDescriptorHeap[BindlessResources.WorldNormalIndex];
    Texture2D MasksBTexture = ResourceDescriptorHeap[BindlessResources.MasksBIndex];
    Texture2D DepthTexture = ResourceDescriptorHeap[BindlessResources.DepthIndex];
    Texture2D SSAOTexture = ResourceDescriptorHeap[BindlessResources.SSAOIndex];
    
    float4 BaseColor = BaseColorTexture.Sample(LinearSampler, IN.UV);
    float3 Transmittance = MasksBTexture.Sample(LinearSampler, IN.UV).rgb;
    float2 WorldNormal = WorldNormalTexture.Sample(LinearSampler, IN.UV).xy;
    float Depth = DepthTexture.Sample(LinearSampler, IN.UV).x;
    
    
    //float4 WorldPos = mul(View.ProjectionToWorld, float4(IN.ScreenPos, Depth, 1));
    //WorldPos.xyz /= WorldPos.w;
    float EE = ConvertFromDeviceZ(Depth, View.InvDeviceZToWorldZTransform);
    float4 WorldPos = mul(View.ScreenToTranslatedWorld, float4(IN.ScreenPos * EE, EE, 1));
    
    float3 N = DecodeNormal(WorldNormal);
    
    GBufferData Gbuffer;
    Gbuffer.BaseColor = BaseColor.xyz;
    Gbuffer.WorldNormal = N;
    Gbuffer.Matallic = Masks.r;
    Gbuffer.Roughness = Masks.g;
    Gbuffer.AmbientOcclusion = Masks.b;
    Gbuffer.TransmittanceColor = Transmittance;
    Gbuffer.ShadingModel = ShadingModel;
    
    float3 CameraVector = View.CameraPos - WorldPos.xyz;
    float3 Radiance = 0;
    float Attenuation = 1;
    float Shadow = 1;
    
    float SSAO = SSAOTexture.Sample(LinearSampler, IN.UV).x;
    float CombinedAO = SSAO * Masks.b;
    
    [branch]
    if (BindlessResources.LightFlags & LIGHT_BITFLAG_POINTLIGHT)
    {
        ConstantBuffer<PointLightData> Light = ResourceDescriptorHeap[BindlessResources.LightDataIndex];
        Radiance = CalculatePointLightRadiance(WorldPos.xyz, Light.WorldPosAndScale.xyz, Light.Color, CameraVector, Gbuffer);
        Attenuation = CalculatePointLightAttenuation(WorldPos.xyz, Light.WorldPosAndScale.xyz, Light.InvRadius);
        
        [branch]
        if(Light.ShadowDataIndex != 0)
        {
            ConstantBuffer<PointLightShadowData> ShadowBuffer = ResourceDescriptorHeap[Light.ShadowDataIndex];
            TextureCube ShadowmapTexture = ResourceDescriptorHeap[ShadowBuffer.ShadowMapTextureIndex];
            SamplerComparisonState CompState = ResourceDescriptorHeap[StaticSamplers.LinearCmpSamplerIndex];
            Shadow = CalculatePointLightShadow(WorldPos.xyz, Light.WorldPosAndScale.xyz, ShadowBuffer.WorldToProjectionMatrices,
                ShadowBuffer.DepthBias, ShadowBuffer.InvShadowmapResolution, ShadowmapTexture, CompState);
        }
    }
    
    else if(BindlessResources.LightFlags & LIGHT_BITFLAG_SPOTLIGHT)
    {
        ConstantBuffer<SpotLightData> Light = ResourceDescriptorHeap[BindlessResources.LightDataIndex];
        Attenuation = CalculateSpotLightAttenuation(WorldPos.xyz, Light.WorldPosition, Light.InvRadius, Light.Direction, Light.CosOuterCone, Light.InvCosConeDifference);
        Radiance = CalculatePointLightRadiance(WorldPos.xyz, Light.WorldPosition, Light.Color, CameraVector, Gbuffer);
        
        [branch]
        if(Light.ShadowDataIndex != 0)
        {
            ConstantBuffer<SpotLightShadowData> ShadowBuffer = ResourceDescriptorHeap[Light.ShadowDataIndex];
            Texture2D ShadowmapTexture = ResourceDescriptorHeap[ShadowBuffer.ShadowMapTextureIndex];
            SamplerComparisonState CompState = ResourceDescriptorHeap[StaticSamplers.LinearCmpSamplerIndex];
            float3 ToLight = Light.WorldPosition - WorldPos.xyz;
            Shadow = CalculateSpotLightShadow(WorldPos.xyz, ToLight, ShadowBuffer.WorldToProjectionMatrix,
                ShadowBuffer.DepthBias, ShadowBuffer.InvShadowmapResolution, ShadowmapTexture, CompState);
        }
    }
    
    else if (BindlessResources.LightFlags & LIGHT_BITFLAG_DIRECTIONAL)
    {
        [branch]
        if (Depth > 0.0001)
        {
            ConstantBuffer<DirectionalLightData> Light = ResourceDescriptorHeap[BindlessResources.LightDataIndex];
            Radiance = CalculateDirectionalLightRadiance(WorldPos.xyz, Light.Direction, Light.Color, CameraVector, Gbuffer);
            
            [branch]
            if (Light.ShadowDataIndex != 0)
            {
                ConstantBuffer<DirectionalLightShadowData> ShadowBuffer = ResourceDescriptorHeap[Light.ShadowDataIndex];
                SamplerComparisonState CompState = ResourceDescriptorHeap[StaticSamplers.LinearCmpSamplerIndex];
                Shadow = CalculateDirectionalLightShadow(WorldPos.xyz, ConvertFromDeviceZ(Depth, View.InvDeviceZToWorldZTransform), Light, ShadowBuffer, CompState);
            }
        }
    }
    
    return float4(Radiance * Attenuation * Shadow * CombinedAO, 1);
}