#include "Common.hlsl"

// SUPPORT_EDITOR_PRIMITIVE_PASS

struct Resources
{
    uint ViewIndex;
    uint PrimitiveIndex;
    uint StaticSamplerBufferIndex;
    uint TextureBufferIndex;
    uint ScalarBufferIndex;
    uint VectorBufferIndex;
};

ConstantBuffer<Resources> BindlessResources : register(b0);

struct View
{
    
};

struct Primitive
{
    matrix LocalToWorld;
    matrix LocalToProjection;
    uint4 Guid;
};

struct StaticSamplers
{
    uint LinearSamplerIndex;
};

struct ScalarBuffer
{
    float MipLevel; // @SCALAR MipLevel
};
    
struct TextureBuffers
{
    uint BaseColorTexture; // @TEXCUBE Texture
};

struct VertexShaderOutput
{
    float2 UV : TEXCOORD0;
    float4 Position : SV_Position;
};

struct PixelShaderOutput
{
#if MAIN_PASS
    float4 ColorDeferred : SV_TARGET0;
    float4 BaseColor : SV_TARGET1;
    float4 WorldNormal : SV_TARGET2;
    float4 Masks : SV_TARGET3;
    float4 MasksB : SV_TARGET4;
    float2 Velocity : SV_TARGET5;
#elif HITPROXY_PASS
    uint4 Guid;
#elif EDITOR_PRIMITIVE_PASS
    float4 Color;
#endif
};

VertexShaderOutput Main_VS(VertexInputStaticMesh IN)
{
    ConstantBuffer<Primitive> PrimitiveBuffer = ResourceDescriptorHeap[BindlessResources.PrimitiveIndex];
    
    VertexShaderOutput OUT;

    OUT.Position = mul(PrimitiveBuffer.LocalToProjection, float4(IN.Position, 1.0f));
    OUT.UV = IN.UV1;
    
    return OUT;
}

// -------------------------------------------------------------------------------------

struct PixelShaderInput
{
    float2 UV : TEXCOORD0;
};

PixelShaderOutput Main_PS(PixelShaderInput IN) : SV_Target
{
    ConstantBuffer<StaticSamplers> StaticSamplers = ResourceDescriptorHeap[BindlessResources.StaticSamplerBufferIndex];
    SamplerState LinearSampler = ResourceDescriptorHeap[StaticSamplers.LinearSamplerIndex];
    
    ConstantBuffer<ScalarBuffer> Scalars = ResourceDescriptorHeap[BindlessResources.ScalarBufferIndex];
    ConstantBuffer<TextureBuffers> Textures = ResourceDescriptorHeap[BindlessResources.TextureBufferIndex];
    TextureCube Texture = ResourceDescriptorHeap[Textures.BaseColorTexture];
    
    const float CPI = 3.14159265;
    
    PixelShaderOutput OUT;
    
    float2 uv = IN.UV;
    
    float2 Angles = float2(2 * CPI * (uv.x + 0.5f), CPI * uv.y);
	
    float s = sin(Angles.y);
    float3 uvw = float3(s * sin(Angles.x), cos(Angles.y), -s * cos(Angles.x));
    
    OUT.Color = Texture.SampleLevel(LinearSampler, uvw, Scalars.MipLevel);
    OUT.Color.a = 1;
    
    return OUT;
}