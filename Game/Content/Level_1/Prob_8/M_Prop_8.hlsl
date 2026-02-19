#include "Common.hlsl"

// SUPPORT_MAIN_PASS
// SUPPORT_PRE_PASS
// SUPPORT_HIT_PROXY_PASS
// SUPPORT_EDITOR_SELECTION_PASS
// SUPPORT_SHADOW_PASS

// HAS_CUSTOM_PRE_PASS
// HAS_OPACITY

struct Resources
{
    uint ViewIndex;
    uint PrimitiveIndex;
    uint StaticSamplerBufferIndex;
    uint ParametersBufferIndex;
    uint unused_1;
    uint unused_2;
    uint ShadowDepthBuffer;
    uint DecalBaseColor;
    uint DecalNormal;
    uint DecalMasks;
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
    float2 Pad_1;
    
    matrix ClipToPreviousClip;

};

struct Primitive
{
    matrix LocalToWorld;
    matrix LocalToProjection;
    uint4 Guid;
    matrix PrevLocalToWorld;
    matrix PrevLocalToProjection;
};

struct StaticSamplers
{
    uint LinearSamplerIndex;
    uint PointSamplerIndex;
};

struct ParametersBuffers
{
    VECTOR(TintColor, TintColor)
    
    SCALAR(RoughnessIntensity, RoughnessIntensity)
    SCALAR(NormalIntensity, NormalIntensity)
    SCALAR(SubsurfaceColorIntensity, SubsurfaceColorIntensity)

    TEX2D(BaseColorOpacity, BaseColorTexture)
    TEX2D(Normal, NormalTexture)
};

struct VectorBuffer
{
    float4 TintColor; // @VECTOR TintColor
};

#if SHADOW_PASS_POINTLIGHT
struct ShadowDepth
{
    matrix WorldToProjectionMatrices[6];
};
#elif SHADOW_PASS_SPOTLIGHT
struct ShadowDepth
{
    matrix WorldToProjectionMatrix;
};
#endif

struct VertexShaderInput
{
    float3 Position : POSITION;
    float2 UV1 : TEXCOORD0;

#if MAIN_PASS
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
#endif
};

struct VertexShaderOutput
{
    float2 UV : TEXCOORD;
    float4 Position : SV_Position;
    
#if MAIN_PASS
    float3x3 TBN : TBN;
#endif
};

VertexShaderOutput Main_VS(VertexShaderInput IN)
{
    VertexShaderOutput OUT;
    

#if SHADOW_PASS_POINTLIGHT
    ConstantBuffer<Primitive> P = ResourceDescriptorHeap[BindlessResources.PrimitiveIndex];
    OUT.Position = mul(P.LocalToWorld, float4(IN.Position, 1.0f));    
#elif SHADOW_PASS_SPOTLIGHT
    ConstantBuffer<Primitive> P = ResourceDescriptorHeap[BindlessResources.PrimitiveIndex];
    ConstantBuffer<ShadowDepth> ShadowBuffer = ResourceDescriptorHeap[BindlessResources.ShadowDepthBuffer];
    float3 WorldPosition = mul(P.LocalToWorld, float4(IN.Position, 1.0f)).xyz;
    OUT.Position = mul(ShadowBuffer.WorldToProjectionMatrix, float4(WorldPosition, 1));
#elif PRE_PASS
    ConstantBuffer<Primitive> P = ResourceDescriptorHeap[BindlessResources.PrimitiveIndex];
    OUT.Position = mul(P.LocalToProjection, float4(IN.Position, 1.0f));
#elif MAIN_PASS
    ConstantBuffer<Primitive> P = ResourceDescriptorHeap[BindlessResources.PrimitiveIndex];
    
    float3 WorldNormal = normalize(mul((float3x3) P.LocalToWorld, IN.Normal));
    float3 WorldTangent = normalize(mul((float3x3) P.LocalToWorld, IN.Tangent));
    OUT.TBN = GetTBN(WorldNormal, WorldTangent);
    
    OUT.Position = mul(P.LocalToProjection, float4(IN.Position, 1.0f));
#else
    ConstantBuffer<Primitive> P = ResourceDescriptorHeap[BindlessResources.PrimitiveIndex];
    OUT.Position = mul(P.LocalToProjection, float4(IN.Position, 1.0f));
#endif
    
    OUT.UV = IN.UV1;
    return OUT;
}

//// -------------------------------------------------------------------------------------

struct PixelShaderInput
{
    float2 UV : TEXCOORD;
    float4 Position : SV_Position;
#if MAIN_PASS
    float3x3 TBN : TBN;
#endif
};

//#define MAIN_PASS 1
//#define PRE_PASS 1

BasePassPixelShaderOutput Main_PS(PixelShaderInput IN, bool FrontFace : SV_IsFrontFace) : SV_Target
{
    BasePassPixelShaderOutput OUT;
 
    ConstantBuffer<StaticSamplers> StaticSamplers = ResourceDescriptorHeap[BindlessResources.StaticSamplerBufferIndex];
    SamplerState LinearSampler = ResourceDescriptorHeap[StaticSamplers.LinearSamplerIndex];
    
    ConstantBuffer<ParametersBuffers> Parameters = ResourceDescriptorHeap[BindlessResources.ParametersBufferIndex];

    Texture2D BaseColorOpacityTexture = ResourceDescriptorHeap[Parameters.BaseColorOpacity_Texture];
    SamplerState BaseColorOpacitySampler = ResourceDescriptorHeap[Parameters.BaseColorOpacity_Sampler];
    
    float4 BaseColorOpacity = BaseColorOpacityTexture.Sample(BaseColorOpacitySampler, IN.UV);
    clip(BaseColorOpacity.a - 0.33f);
    
#if MAIN_PASS

    Texture2D NormalTexture = ResourceDescriptorHeap[Parameters.Normal_Texture];
    SamplerState NormalSampler = ResourceDescriptorHeap[Parameters.Normal_Sampler];
    
    float3 Masks = float3(0, Parameters.RoughnessIntensity, 1);
    
    float3 Normal = NormalTexture.Sample(NormalSampler, IN.UV).rgb;
    Normal = ReconstructTextureNormal(Normal.xy);

    Normal = lerp( float3(0.0f, 1.0f, 0.0f), Normal, Parameters.NormalIntensity );
    Normal = normalize(mul(Normal, IN.TBN));
    if(!FrontFace)
    {
        Normal = -Normal;
    }
    float2 N = EncodeNormal(Normal);
    
    OUT.ColorDeferred = float4(0.0, 0.0, 0.0, 1);
    OUT.BaseColor = float4(BaseColorOpacity.rgb, 1);
    OUT.WorldNormal = N;
    OUT.Masks = float4(Masks, Uint8ToFloat(SHADING_MODEL_FOLIAGE));
    
    OUT.MasksB = float4(BaseColorOpacity.rgb * Parameters.SubsurfaceColorIntensity, 1);
    
    
    //float2 Velocity = IN.ScreenPos.xy / IN.ScreenPos.w - IN.PrevScreenPos.xy / IN.PrevScreenPos.w;
    //Velocity = Velocity * 0.25f + 0.5f;
    //OUT.Velocity = Velocity;
    //OUT.Velocity = 0;
    
    
#elif HITPROXY_PASS
    ConstantBuffer<Primitive> P = ResourceDescriptorHeap[BindlessResources.PrimitiveIndex];
    OUT.Guid = P.Guid;

#elif PRE_PASS || SHADOW_PASS
#endif
    
    return OUT;
}

// -------------------------------------------------------------------------------------

struct GeometeryShaderOutput
{
    float2 UV : TEXCOORD;
    float4 Position : SV_Position;
    uint TargetIndex : SV_RenderTargetArrayIndex;
};

#if SHADOW_PASS_POINTLIGHT

[maxvertexcount(18)]
void PointLightShadow_GS(triangle VertexShaderOutput input[3], inout TriangleStream<GeometeryShaderOutput> OutputStream)
{
    ConstantBuffer<ShadowDepth> ShadowDepthBuffer = ResourceDescriptorHeap[BindlessResources.ShadowDepthBuffer];
    
    [unroll]
    for (int CubeFaceIndex = 0; CubeFaceIndex < 6; CubeFaceIndex++)
    {
        [unroll]
		for (int VertexIndex = 0; VertexIndex < 3; VertexIndex++)
		{
            GeometeryShaderOutput OUT;
            OUT.TargetIndex = CubeFaceIndex;

            float3 WorldPosition = input[VertexIndex].Position.xyz;
            OUT.Position = mul(ShadowDepthBuffer.WorldToProjectionMatrices[CubeFaceIndex], float4(WorldPosition, 1.0f));
            OUT.UV = input[VertexIndex].UV;
            OutputStream.Append(OUT);
        }
		OutputStream.RestartStrip();
    }
}

#endif