//#include "../../../Engine/Content/Materials/Common.hlsl"

// SUPPORT_MAIN_PASS
// SUPPORT_PRE_PASS

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

struct VectorBuffer
{
    float4 ShowColor; // @VECTOR ShowColor
};

struct TextureBuffers
{
    uint BaseColorTexture; // @TEX2D Texture
};

struct VertexInputStaticMesh
{
    float3 Position : POSITION;
    float3 Color : COLOR;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 Bitangent : BINORMAL;
    float2 UV1 : TEXCOORD0;
    float2 UV2 : TEXCOORD1;
    float2 UV3 : TEXCOORD2;
    float2 UV4 : TEXCOORD3;
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
    
    ConstantBuffer<TextureBuffers> Textures = ResourceDescriptorHeap[BindlessResources.TextureBufferIndex];
    Texture2D Texture = ResourceDescriptorHeap[Textures.BaseColorTexture];
    
    ConstantBuffer<ScalarBuffer> Scalars = ResourceDescriptorHeap[BindlessResources.ScalarBufferIndex];
    ConstantBuffer<VectorBuffer> Vectors = ResourceDescriptorHeap[BindlessResources.VectorBufferIndex];
    
    PixelShaderOutput OUT;
    float4 Sample = Texture.SampleLevel(LinearSampler, IN.UV, Scalars.MipLevel);
    
    //OUT.ColorDeferred = float4(BaseColor, 1);
    //OUT.ColorDeferred = pow(OUT.ColorDeferred, 1.0f / 2.2f);
    OUT.BaseColor = 0;
    OUT.WorldNormal = 0;
    OUT.Masks = 0;
    //OUT.Masks.a = 1.0f/255;
    OUT.Masks.a = 0;
    
    if(Vectors.ShowColor.a > 0)
    {
        OUT.ColorDeferred = float4(Sample.aaa, 1);
    }
    else
    {
        OUT.ColorDeferred = float4(Sample.rgb, 1);
    }
    
    return OUT;
}