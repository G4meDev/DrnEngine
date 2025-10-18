#include "Common.hlsl"

struct Resources
{
    uint ViewIndex;
    uint DecalBufferIndex;
    uint StaticSamplerBufferIndex;
    uint TextureBufferIndex;
    uint ScalarBufferIndex;
    uint VectorBufferIndex;
    uint ShadowDepthBuffer;
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
    float2 JitterOffset;
    
    float2 PrevJitterOffset;
};

struct DecalBuffer
{
    matrix LocalToProjection;
    matrix ProjectionToLocal;
};

struct StaticSamplers
{
    uint LinearSamplerIndex;
};

struct TextureBuffers
{
    uint BaseColorTexture; // @TEX2D BaseColorTexture
    uint NormalTexture; // @TEX2D NormalTexture
    uint MasksTexture; // @TEX2D MasksTexture
};

struct ScalarBuffer
{
    float TintIntensity; // @SCALAR ColorIntensity
    float RoughnessIntensity; // @SCALAR RoughnessIntensity
    float NormalIntensity; // @SCALAR NormalIntensity
};

struct VectorBuffer
{
    float4 TintColor; // @VECTOR TintColor
};

struct VertexShaderOutput
{
    float4 Position : SV_Position;
};

VertexShaderOutput Main_VS(VertexInputPositionOnly IN)
{
    VertexShaderOutput OUT;
    
    ConstantBuffer<DecalBuffer> Decal = ResourceDescriptorHeap[BindlessResources.DecalBufferIndex];
    
    OUT.Position = mul(Decal.LocalToProjection, float4(IN.Position, 1.0f));
    return OUT;
}

//// -------------------------------------------------------------------------------------

struct PixelShaderInput
{
    float4 Position : SV_Position;
};

struct PixelShaderOutput
{
    float4 BaseColor : SV_TARGET0;
    float4 Normal : SV_TARGET1;
    float4 Masks : SV_TARGET2;
};

PixelShaderOutput Main_PS(PixelShaderInput IN) : SV_Target
{
    PixelShaderOutput OUT;

    ConstantBuffer<StaticSamplers> StaticSamplers = ResourceDescriptorHeap[BindlessResources.StaticSamplerBufferIndex];
    SamplerState LinearSampler = ResourceDescriptorHeap[StaticSamplers.LinearSamplerIndex];
    
    ConstantBuffer<TextureBuffers> Textures = ResourceDescriptorHeap[BindlessResources.TextureBufferIndex];
    Texture2D BaseColorTexture = ResourceDescriptorHeap[Textures.BaseColorTexture];
    Texture2D NormalTexture = ResourceDescriptorHeap[Textures.NormalTexture];
    Texture2D MasksTexture = ResourceDescriptorHeap[Textures.MasksTexture];
    
    //float3 BaseColor = BaseColorTexture.Sample(LinearSampler, IN.UV).xyz;
    //float3 Masks = MasksTexture.Sample(LinearSampler, IN.UV).xyz;

    ConstantBuffer<ScalarBuffer> Scalars = ResourceDescriptorHeap[BindlessResources.ScalarBufferIndex];
    ConstantBuffer<VectorBuffer> Vectors = ResourceDescriptorHeap[BindlessResources.VectorBufferIndex];
    
    OUT.BaseColor = 1.0f;
    OUT.Normal = 1.0f;
    OUT.Masks = 1.0f;
    
    return OUT;
}