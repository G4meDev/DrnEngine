#include "Common.hlsl"

// SUPPORT_DEFERRED_DECAL_PASS

struct Resources
{
    uint ViewIndex;
    uint DecalBufferIndex;
    uint StaticSamplerBufferIndex;
    uint TextureBufferIndex;
    uint ScalarBufferIndex;
    uint VectorBufferIndex;
    uint Empty;
    uint DepthTexture;
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
    uint PointSamplerIndex;
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

    ConstantBuffer<ViewBuffer> View = ResourceDescriptorHeap[BindlessResources.ViewIndex];
    ConstantBuffer<DecalBuffer> Decal = ResourceDescriptorHeap[BindlessResources.DecalBufferIndex];
    ConstantBuffer<StaticSamplers> StaticSamplers = ResourceDescriptorHeap[BindlessResources.StaticSamplerBufferIndex];
    SamplerState LinearSampler = ResourceDescriptorHeap[StaticSamplers.LinearSamplerIndex];
    SamplerState PointSampler = ResourceDescriptorHeap[StaticSamplers.PointSamplerIndex];
    
    Texture2D DepthTexture = ResourceDescriptorHeap[BindlessResources.DepthTexture];

    float2 ScreenUV = SvPositionToViewportUV(IN.Position.xy, View.InvSize);
    float PixelDepth = DepthTexture.Sample(PointSampler, ScreenUV).x;
    float4 ClipPosition = float4(ViewportUVToScreenPos(ScreenUV), PixelDepth, 1);
    
    float4 LocalPosition = mul(Decal.ProjectionToLocal, ClipPosition);
    LocalPosition.xyz /= LocalPosition.w;
    
    clip(LocalPosition.xyz + 1.0f);
    clip(1.0f - LocalPosition.xyz);
    
    ConstantBuffer<TextureBuffers> Textures = ResourceDescriptorHeap[BindlessResources.TextureBufferIndex];
    ConstantBuffer<ScalarBuffer> Scalars = ResourceDescriptorHeap[BindlessResources.ScalarBufferIndex];
    ConstantBuffer<VectorBuffer> Vectors = ResourceDescriptorHeap[BindlessResources.VectorBufferIndex];

    Texture2D BaseColorTexture = ResourceDescriptorHeap[Textures.BaseColorTexture];
    Texture2D NormalTexture = ResourceDescriptorHeap[Textures.NormalTexture];
    Texture2D MasksTexture = ResourceDescriptorHeap[Textures.MasksTexture];
    
    float2 DecalUVs = LocalPosition.xz * float2(0.5f, -0.5f) + 0.5f;
    
    float3 BaseColor = BaseColorTexture.Sample(LinearSampler, DecalUVs).xyz;
    float3 Normal = NormalTexture.Sample(LinearSampler, DecalUVs).xyz;
    float3 Masks = MasksTexture.Sample(LinearSampler, DecalUVs).xyz;
    
    float BlendAlpha = 1.0f;
    BlendAlpha *= 4 * (1 - abs(LocalPosition.y));
    BlendAlpha *= Masks.r;
    
    OUT.BaseColor = float4(BaseColor, BlendAlpha);
    OUT.Normal = float4(Normal, BlendAlpha);
    OUT.Masks = float4(0.0f, Masks.gb, BlendAlpha);
    
    return OUT;
}