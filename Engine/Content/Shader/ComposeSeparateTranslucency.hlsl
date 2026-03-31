
#include "Common.hlsl"

struct Resources
{
    uint ViewBufferIndex;
    uint Unused;
    uint StaticSamplerBufferIndex;
    uint DeferredColorTextureIndex;
    uint SeparateTranslucencyTextureIndex;
};

ConstantBuffer<Resources> BindlessResources : register(b0);

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

float4 Main_PS(PixelShaderInput IN) : SV_Target
{
    ConstantBuffer<StaticSamplers> StaticSamplers = ResourceDescriptorHeap[BindlessResources.StaticSamplerBufferIndex];
    SamplerState PointSampler = ResourceDescriptorHeap[StaticSamplers.PointSamplerIndex];
    
    Texture2D DeferredColorTexture = ResourceDescriptorHeap[BindlessResources.DeferredColorTextureIndex];
    Texture2D SeparateTranslucencyTexture = ResourceDescriptorHeap[BindlessResources.SeparateTranslucencyTextureIndex];
    
    float4 DeferredColor = DeferredColorTexture.Sample(PointSampler, IN.UV);
    float4 SeparateTranslucency = SeparateTranslucencyTexture.Sample(PointSampler, IN.UV);
    
    float3 Result = DeferredColor.rgb * SeparateTranslucency.a + SeparateTranslucency.rgb;
    
    return float4(Result, 0);
}