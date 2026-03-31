
#include "Common.hlsl"

static const half InvDistortionScaleBias = 1 / 4.0f;

struct Resources
{
    uint ViewBufferIndex;
    uint Unused;
    uint StaticSamplerBufferIndex;
    uint DeferredColorTextureIndex;
    uint DistortionTextureIndex;
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
    Texture2D DistortionTexture = ResourceDescriptorHeap[BindlessResources.DistortionTextureIndex];
    
    half4 AccumDist = DistortionTexture.Sample(PointSampler, IN.UV);
    half2 DistBufferUVOffset = (AccumDist.rg - AccumDist.ba);
    DistBufferUVOffset *= InvDistortionScaleBias;
    
    float2 NewBufferUV = IN.UV + DistBufferUVOffset;

    [flatten]
    if (NewBufferUV.x < 0 || NewBufferUV.x > 1 || NewBufferUV.y < 0 || NewBufferUV.y > 1)
    {
        NewBufferUV = IN.UV;
    }
    
    float4 Result = DeferredColorTexture.Sample(PointSampler, NewBufferUV);
    return Result;
}