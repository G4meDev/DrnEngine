#include "Common.hlsl"

// SUPPORT_MAIN_PASS
// SUPPORT_PRE_PASS
// SUPPORT_HIT_PROXY_PASS
// SUPPORT_EDITOR_SELECTION_PASS

struct Resources
{
    uint ViewIndex;
    uint PrimitiveIndex;
    uint StaticSamplerBufferIndex;
    uint ParametersBufferIndex;
    uint unused_1;
    uint unused_2;
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

struct ParametersBuffers
{
    VECTOR(LightDir, LightDir)
    VECTOR(ExampleVec4, ExampleVec4)
    
    SCALAR(Alpha, Alpha)
    SCALAR(Rand, Rand)
    SCALAR(WERWER, WERWER)
    
    TEX2D(TestTexture, TestTexture)
    TEX2D(TestTexture_2, TestTexture_2)
};

struct VertexShaderOutput
{
    float4 Color : COLOR0;
    float3 Normal : COLOR1;
    float2 UV : TEXCOORD0;
    float4 Position : SV_Position;
};

VertexShaderOutput Main_VS(VertexInputStaticMesh IN)
{
    ConstantBuffer<Primitive> PrimitiveBuffer = ResourceDescriptorHeap[BindlessResources.PrimitiveIndex];
    
    VertexShaderOutput OUT;

    OUT.Position = mul(PrimitiveBuffer.LocalToProjection, float4(IN.Position, 1.0f));
    OUT.Color = float4(IN.UV1, 0.0f, 1.0f);
    OUT.Normal = IN.Normal;
    OUT.UV = IN.UV1;
    
    return OUT;
}

struct PixelShaderOutput
{
#if MAIN_PASS
    float4 ColorDeferred : SV_TARGET0;
    float4 BaseColor : SV_TARGET1;
    float4 WorldNormal : SV_TARGET2;
    float4 Masks : SV_TARGET3;
    float4 MasksB : SV_TARGET4;
#elif HITPROXY_PASS
    uint4 Guid;
#elif EDITOR_PRIMITIVE_PASS
    float4 Color;
#endif
};

// -------------------------------------------------------------------------------------

struct PixelShaderInput
{
    float4 Color : COLOR0;
    float3 Normal : COLOR1;
    float2 UV : TEXCOORD0;
};

PixelShaderOutput Main_PS(PixelShaderInput IN) : SV_Target
{
    ConstantBuffer<Primitive> PrimitiveBuffer = ResourceDescriptorHeap[BindlessResources.PrimitiveIndex];

    ConstantBuffer<StaticSamplers> StaticSamplers = ResourceDescriptorHeap[BindlessResources.StaticSamplerBufferIndex];
    SamplerState LinearSampler = ResourceDescriptorHeap[StaticSamplers.LinearSamplerIndex];
    
    ConstantBuffer<ParametersBuffers> Parameters = ResourceDescriptorHeap[BindlessResources.ParametersBufferIndex];

    Texture2D TestTexture = ResourceDescriptorHeap[Parameters.TestTexture_Texture];
    SamplerState TestSampler = ResourceDescriptorHeap[Parameters.TestTexture_Sampler];
    
    PixelShaderOutput OUT;

    float4 Texture1 = TestTexture.Sample(TestSampler, IN.UV);
    float L = max(0, dot(IN.Normal, Parameters.LightDir.xyz));
    float4 Result = Texture1 * L * Parameters.Alpha;
    
#if MAIN_PASS
    OUT.ColorDeferred = Result;
    OUT.BaseColor = float4(0.7, 0.5, 1, 1);
    OUT.WorldNormal = float4(0, 1, 0, 1);
    OUT.Masks = float4(0.2, 1, 0.4, 1);
#elif HITPROXY_PASS
    OUT.Guid = PrimitiveBuffer.Guid;
#endif

    return OUT;
}