#include "../Materials/Common.hlsl"

struct VertexInputPosUV
{
    float3 Position : POSITION;
};

struct LightConstantBuffer
{
    matrix LocalToProjection;
    matrix ProjectionToWorld;
    float3 Position;
    float Radius;
    float3 Color;
    float InvRadius;
};

ConstantBuffer<LightConstantBuffer> CB : register(b0);

Texture2D BaseColorTexture : register(t0);
Texture2D WorldNormalTexture : register(t1);
Texture2D MasksTexture : register(t2);
Texture2D DepthTexture : register(t3);

SamplerState TextureSampler : register(s0);

struct VertexShaderOutput
{
    noperspective float2 UV : TEXCOORD;
    noperspective float2 ScreenPos : TEXCOORD1;
    float4 Position : SV_Position;
};

VertexShaderOutput Main_VS(VertexInputPosUV IN)
{
    VertexShaderOutput OUT;

    OUT.Position = mul(CB.LocalToProjection, float4(IN.Position, 1.0f));
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
    float4 BaseColor = BaseColorTexture.Sample(TextureSampler, IN.UV);
    float3 WorldNormal = WorldNormalTexture.Sample(TextureSampler, IN.UV).xyz;
    float4 Masks = MasksTexture.Sample(TextureSampler, IN.UV);
    float Depth = DepthTexture.Sample(TextureSampler, IN.UV).x;

    // TODO: reconstruct pos from camerapos + pixeldir * depth
    float4 WorldPos = mul(CB.ProjectionToWorld, float4(IN.ScreenPos, Depth, 1));
    WorldPos.xyz /= WorldPos.w;
    
    float3 N = DecodeNormal(WorldNormal);
    
    float3 ToLight = CB.Position - WorldPos.xyz;
    float DistanceSquare = dot(ToLight, ToLight);
    
    float3 L = ToLight * rsqrt(DistanceSquare);
    float NoL = saturate(dot( N, L ));

    float Distance = distance(WorldPos.xyz, CB.Position);
    float Attenuation = 1 - Distance / CB.Radius;
    Attenuation = saturate(Attenuation);

    
    
    float3 Result = CB.Color * Attenuation * NoL;
    
    return float4(Result, 1);
    
    //return BaseColor + WorldNormal + Masks + Depth;
    //return WorldNormal;
}