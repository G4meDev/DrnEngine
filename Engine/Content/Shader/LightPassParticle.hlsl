#include "Common.hlsl"

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

struct VertexInputParticleLights
{
    float3 Position : POSITION;
    float4 WorldPositionAndRadius : POS_RADIUS;
    float4 ColorAndInvRadius : COLOR_INVRADIUS;
};

struct VertexShaderOutput
{
    noperspective float2 UV : TEXCOORD;
    noperspective float2 ScreenPos : TEXCOORD1;
    float3 WorldPosition : WORLD_POSITION;
    float3 Color : COLOR;
    float InvRadius : INV_RADIUS;
    float4 Position : SV_Position;
};

VertexShaderOutput Main_VS(VertexInputParticleLights IN)
{
    VertexShaderOutput OUT;
    ConstantBuffer<ViewBuffer> View = ResourceDescriptorHeap[BindlessResources.ViewBufferIndex];

    float3 WorldPosition = IN.Position * IN.WorldPositionAndRadius.w + IN.WorldPositionAndRadius.xyz;
    OUT.Position = mul(View.WorldToProjection, float4(WorldPosition, 1.0f));

    OUT.UV = VSPosToScreenUV(OUT.Position);
    OUT.ScreenPos = OUT.Position.xy / OUT.Position.w;
    
    OUT.WorldPosition = IN.WorldPositionAndRadius.xyz;
    OUT.Color = IN.ColorAndInvRadius.xyz;
    OUT.InvRadius = IN.ColorAndInvRadius.w;
    
    return OUT;
}

struct PixelShaderInput
{
    noperspective float2 UV : TEXCOORD;
    noperspective float2 ScreenPos : TEXCOORD1;
    float3 WorldPosition : WORLD_POSITION;
    float3 Color : COLOR;
    float InvRadius : INV_RADIUS;
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
    
    float SSAO = SSAOTexture.Sample(LinearSampler, IN.UV).x;
    float CombinedAO = SSAO * Masks.b;
    
    float3 Radiance = CalculatePointLightRadiance(WorldPos.xyz, IN.WorldPosition, IN.Color, CameraVector, Gbuffer);
    float Attenuation = CalculatePointLightAttenuation(WorldPos.xyz, IN.WorldPosition, IN.InvRadius);
    
    return float4(Radiance * Attenuation * CombinedAO, 1);
}