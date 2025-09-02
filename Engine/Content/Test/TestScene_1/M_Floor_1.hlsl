//#include "../../../Engine/Content/Materials/Common.hlsl"

// SUPPORT_MAIN_PASS
// SUPPORT_HIT_PROXY_PASS
// SUPPORT_EDITOR_SELECTION_PASS
// SUPPORT_SHADOW_PASS

float2 EncodeNormal(float3 N)
{
    N.xy /= dot(1, abs(N));
    if (N.z <= 0)
    {
        N.xy = (1 - abs(N.yx)) * select(N.xy >= 0, float2(1, 1), float2(-1, -1));
    }
    return N.xy;
}

struct Resources
{
    uint ViewIndex;
    uint PrimitiveIndex;
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

struct TextureBuffers
{
    uint FloorAlbedo; // @TEX2D FloorAlbedo
    uint FloorNormal; // @TEX2D FloorNormal
    uint FloorMasks; // @TEX2D FloorMasks
    
    uint GrassAlbedo; // @TEX2D GrassAlbedo
    uint GrassNormal; // @TEX2D GrassNormal
    uint GrassMasks; // @TEX2D GrassMasks
};

struct ScalarBuffer
{
    float RoughnessIntensity; // @SCALAR RoughnessIntensity
    float NormalIntensity; // @SCALAR NormalIntensity

    float FloorAlpha; // @SCALAR FloorAlpha
    float GrassAlpha; // @SCALAR GrassAlpha
    float Depth; // @SCALAR Depth
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
    float4 Position : SV_Position;
    float4 Color : COLOR;
    float3 Normal : NORMAL;
    float3x3 TBN : TBN;
    float2 UV : TEXCOORD;
};

VertexShaderOutput Main_VS(VertexInputStaticMesh IN)
{
    VertexShaderOutput OUT;

#if SHADOW_PASS_POINTLIGHT
    ConstantBuffer<Primitive> P = ResourceDescriptorHeap[BindlessResources.PrimitiveIndex];
    OUT.Position = mul(P.LocalToWorld, float4(IN.Position, 1.0f));
    
    OUT.TBN = float3x3(IN.Position,IN.Position,IN.Position);
    OUT.Color = float4(IN.Color, 1.0f);
    OUT.Normal = IN.Position;
    OUT.UV = IN.UV1;
#elif SHADOW_PASS_SPOTLIGHT
    ConstantBuffer<Primitive> P = ResourceDescriptorHeap[BindlessResources.PrimitiveIndex];
    ConstantBuffer<ShadowDepth> ShadowBuffer = ResourceDescriptorHeap[BindlessResources.ShadowDepthBuffer];
    float3 WorldPosition = mul(P.LocalToWorld, float4(IN.Position, 1.0f)).xyz;
    OUT.Position = mul(ShadowBuffer.WorldToProjectionMatrix, float4(WorldPosition, 1));

    OUT.TBN = float3x3(IN.Position,IN.Position,IN.Position);
    OUT.Color = float4(IN.Color, 1.0f);
    OUT.Normal = IN.Position;
    OUT.UV = IN.UV1;
#else
    
    ConstantBuffer<Primitive> P = ResourceDescriptorHeap[BindlessResources.PrimitiveIndex];
    
    float3 VertexNormal = normalize(mul((float3x3) P.LocalToWorld, IN.Normal));
    float3 VertexTangent = normalize(mul((float3x3) P.LocalToWorld, IN.Tangent));
    float3 VertexBiNormal = normalize(mul((float3x3) P.LocalToWorld, IN.Bitangent));
    //OUT.TBN = float3x3(VertexTangent, VertexBiNormal, VertexNormal);
    OUT.TBN = float3x3(VertexTangent, VertexNormal, VertexBiNormal);
    
    OUT.Position = mul(P.LocalToProjection, float4(IN.Position, 1.0f));
    OUT.Color = float4(IN.Color, 1.0f);
    OUT.Normal = VertexNormal;
    OUT.UV = IN.UV1;
    
#endif
    
    return OUT;
}

//// -------------------------------------------------------------------------------------

struct PixelShaderInput
{
#if SHADOW_PASS
    float4 Position : SV_Position;
#else
    float4 Position : SV_Position;
    float4 Color : COLOR;
    float3 Normal : NORMAL;
    float3x3 TBN : TBN;
    float2 UV : TEXCOORD;
#endif
};

struct PixelShaderOutput
{
#if MAIN_PASS
    float4 ColorDeferred : SV_TARGET0;
    float4 BaseColor : SV_TARGET1;
    float2 WorldNormal : SV_TARGET2;
    float4 Masks : SV_TARGET3;
#elif HITPROXY_PASS
    uint4 Guid;
#elif EDITOR_PRIMITIVE_PASS
    float4 Color;
#elif SHADOW_PASS
#endif
};

float3 ReconstructNormals(float2 xy)
{
    xy.y = 1 - xy.y;
    float2 normalxy = xy * 2 - 1;
    return float3(normalxy.x, 1 - dot(normalxy, normalxy), normalxy.y);
}

float InterleavedGradientNoise(float2 uv, float FrameId)
{
    uv += FrameId * (float2(47, 17) * 0.695f);

    const float3 magic = float3(0.06711056f, 0.00583715f, 52.9829189f);
    return frac(magic.z * frac(dot(uv, magic.xy)));
}

struct LayerData
{
    float3 Color;
    float3 Normal;
    float Roughness;
    float AO;
};

LayerData LayerBlend(LayerData Layer1, LayerData Layer2, float a)
{
    LayerData Result;
    Result.Color = lerp(Layer1.Color, Layer2.Color, a);
    Result.Normal = lerp(Layer1.Normal, Layer2.Normal, a);
    Result.Roughness = lerp(Layer1.Roughness, Layer2.Roughness, a);
    Result.AO = lerp(Layer1.AO, Layer2.AO, a);
    
    return Result;
}

LayerData LayerHeightBlend(LayerData L1, float H1, float A1, LayerData L2, float H2, float A2, float Depth)
{
    LayerData Result;
    
    float ma = max(H1 + A1, H2 + A2) - Depth;
    
    float b1 = saturate(H1 + A1 - ma);
    float b2 = saturate(H2 + A2 - ma);
    
    float s = b1 + b2;
    
    Result.Color = (L1.Color * b1 + L2.Color * b2) / s;
    Result.Normal = (L1.Normal * b1 + L2.Normal * b2) / s;
    Result.Roughness = (L1.Roughness * b1 + L2.Roughness * b2) / s;
    Result.AO = (L1.AO * b1 + L2.AO * b2) / s;
    
    return Result;
}

float ClampRange(float Input, float Minimum, float Maximum)
{
    return saturate((Input - Minimum) / (Maximum - Minimum));
}

//#define MAIN_PASS 1

PixelShaderOutput Main_PS(PixelShaderInput IN) : SV_Target
{
    PixelShaderOutput OUT;
 
#if MAIN_PASS

    ConstantBuffer<ScalarBuffer> Scalars = ResourceDescriptorHeap[BindlessResources.ScalarBufferIndex];
    ConstantBuffer<VectorBuffer> Vectors = ResourceDescriptorHeap[BindlessResources.VectorBufferIndex];

    ConstantBuffer<ViewBuffer> View = ResourceDescriptorHeap[BindlessResources.ViewIndex];
    ConstantBuffer<StaticSamplers> StaticSamplers = ResourceDescriptorHeap[BindlessResources.StaticSamplerBufferIndex];
    SamplerState LinearSampler = ResourceDescriptorHeap[StaticSamplers.LinearSamplerIndex];
    
    ConstantBuffer<TextureBuffers> Textures = ResourceDescriptorHeap[BindlessResources.TextureBufferIndex];
    Texture2D FloorAlbedoTexture = ResourceDescriptorHeap[Textures.FloorAlbedo];
    Texture2D FloorNormalTexture = ResourceDescriptorHeap[Textures.FloorNormal];
    Texture2D FloorMasksTexture = ResourceDescriptorHeap[Textures.FloorMasks];
    
    Texture2D GrassAlbedoTexture = ResourceDescriptorHeap[Textures.GrassAlbedo];
    Texture2D GrassNormalTexture = ResourceDescriptorHeap[Textures.GrassNormal];
    Texture2D GrassMasksTexture = ResourceDescriptorHeap[Textures.GrassMasks];

    float4 VertexColor = IN.Color;
    
    float3 FloorColor = FloorAlbedoTexture.Sample(LinearSampler, IN.UV).xyz;
    float3 FloorNormal = FloorNormalTexture.Sample(LinearSampler, IN.UV).xyz;
    float4 FloorMasks = FloorMasksTexture.Sample(LinearSampler, IN.UV);
    
    LayerData FloorLayer;
    FloorLayer.Color = FloorColor;
    FloorLayer.Normal = FloorNormal;
    FloorLayer.Roughness = FloorMasks.g;
    FloorLayer.AO = FloorMasks.b;
    
    float3 GrassColor = GrassAlbedoTexture.Sample(LinearSampler, IN.UV).xyz;
    float3 GrassNormal = GrassNormalTexture.Sample(LinearSampler, IN.UV).xyz;
    float4 GrassMasks = GrassMasksTexture.Sample(LinearSampler, IN.UV);

    LayerData GrassLayer;
    GrassLayer.Color = GrassColor;
    GrassLayer.Normal = GrassNormal;
    GrassLayer.Roughness = GrassMasks.g;
    GrassLayer.AO = GrassMasks.b;
    
    LayerData BlendLayer;
    //BlendLayer = LayerBlend(FloorLayer, GrassLayer, VertexColor.r);
    //BlendLayer = LayerHeightBlend(FloorLayer, FloorMasks.r, Scalars.FloorAlpha , GrassLayer, GrassMasks.r, Scalars.GrassAlpha, Scalars.Depth);
    BlendLayer = LayerHeightBlend(FloorLayer, FloorMasks.r, 1 - VertexColor.r , GrassLayer, GrassMasks.r, VertexColor.r, Scalars.Depth);
    
    float3 BaseColor = BlendLayer.Color;
    float3 Masks = float3(0, BlendLayer.Roughness, BlendLayer.AO);
    float3 Normal = BlendLayer.Normal;

    float WetnessMask = VertexColor.y;
    float3 BaseColorSq = BaseColor * BaseColor;
    BaseColor = lerp(BaseColor, BaseColorSq, ClampRange(WetnessMask, 0, 0.35));
    Masks.g = lerp(Masks.g, 0.1, ClampRange(WetnessMask, 0.2, 1.0));
    Masks.b = lerp(Masks.b, 1.0, ClampRange(WetnessMask, 0.45, 0.95));
    //Normal = lerp(Normal, float3(0.5, 0.5, 1.0), ClampRange(WetnessMask, 0.45, 0.95));
    
    //float3 Normal = float3(0.5, 0.5, 1);
    //float3 Normal = FloorNormalTexture.Sample(LinearSampler, IN.UV).rgb;
    Masks.g *= Scalars.RoughnessIntensity;

    Normal = ReconstructNormals(Normal.xy);
    Normal = lerp( float3(0.0f, 1.0f, 0.0f), Normal, Scalars.NormalIntensity );
    Normal = normalize(mul(Normal, IN.TBN));
    
    Normal = lerp(Normal, float3(0, 1, 0), ClampRange(WetnessMask, 0.45, 0.95));
    
    //float DitherNoise = InterleavedGradientNoise(IN.Position.xy, View.FrameIndexMod8);
    
    OUT.ColorDeferred = float4(0.0, 0.0, 0.0, 1);
    OUT.BaseColor = float4(BaseColor, 1);
    //OUT.BaseColor = lerp(OUT.BaseColor, Vectors.TintColor, Scalars.TintIntensity);
    OUT.WorldNormal = EncodeNormal(Normal);
    OUT.Masks = float4(Masks, 1.0f/255);
    
#elif HITPROXY_PASS
    ConstantBuffer<Primitive> P = ResourceDescriptorHeap[BindlessResources.PrimitiveIndex];
    OUT.Guid = P.Guid;
#endif
    
    return OUT;
}

// -------------------------------------------------------------------------------------

struct GeometeryShaderOutput
{
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
            OutputStream.Append(OUT);
        }
		OutputStream.RestartStrip();
    }
}

#endif

//ConstantBuffer<ViewBuffer> View : register(b0);
//
//Texture2D BaseColorTexture : register(t0);
//SamplerState BaseColorSampler : register(s0);
//
//Texture2D NormalTexture : register(t1);
//SamplerState NormalSampler : register(s1);
//
//Texture2D MasksTexture : register(t2);
//SamplerState MasksSampler : register(s2);
//
//struct VertexShaderOutput
//{
//    float4 Color : COLOR;
//    float3 Normal : NORMAL;
//    float3x3 TBN : TBN;
//    float2 UV : TEXCOORD;
//    float4 Position : SV_Position;
//};
//
//VertexShaderOutput Main_VS(VertexInputStaticMesh IN)
//{
//#if SHADOW_PASS
//    
//    // TODO: simplify by macro for point light
//    
//#endif
//    VertexShaderOutput OUT;
//
//    float3 VertexNormal = normalize(mul((float3x3) View.LocalToWorld, IN.Normal));
//    float3 VertexTangent = normalize(mul((float3x3) View.LocalToWorld, IN.Tangent));
//    float3 VertexBiNormal = normalize(mul((float3x3) View.LocalToWorld, IN.Bitangent));
//    //OUT.TBN = float3x3(VertexTangent, VertexBiNormal, VertexNormal);
//    OUT.TBN = float3x3(VertexTangent, VertexNormal, VertexBiNormal);
//    
//    OUT.Position = mul(View.LocalToProjection, float4(IN.Position, 1.0f));
//    OUT.Color = float4(IN.Color, 1.0f);
//    OUT.Normal = VertexNormal;
//    OUT.UV = IN.UV1;
//    
//    return OUT;
//}
//
//// -------------------------------------------------------------------------------------
//
//struct PixelShaderInput
//{
//    float4 Color : COLOR;
//    float3 Normal : NORMAL;
//    float3x3 TBN : TBN;
//    float2 UV : TEXCOORD;
//};
//
//PixelShaderOutput Main_PS(PixelShaderInput IN) : SV_Target
//{
//    PixelShaderOutput OUT;
//
//#if MAIN_PASS
//    float3 BaseColor = BaseColorTexture.Sample(BaseColorSampler, IN.UV).xyz;
//    float3 Masks = MasksTexture.Sample(MasksSampler, IN.UV).xyz;
//    
//    float3 Normal = NormalTexture.Sample(NormalSampler, IN.UV).rbg;
//    Normal.z = 1 - Normal.z;
//    Normal = Normal * 2 - 1;
//    Normal = normalize(mul(Normal, IN.TBN));
//    Normal = EncodeNormal(Normal);
//    
//    OUT.ColorDeferred = float4(0.0, 0.0, 0.0, 1);
//    OUT.BaseColor = float4(BaseColor, 1);
//    OUT.WorldNormal = float4( Normal, 0);
//    OUT.Masks = float4(Masks, 1);
//#elif HitProxyPass
//    OUT.Guid = View.Guid;
//#endif
//    
//    return OUT;
//}
//
//// -------------------------------------------------------------------------------------
//
//struct GeometeryShaderOutput
//{
//    float4 Position : SV_Position;
//    uint TargetIndex : SV_RenderTargetArrayIndex;
//};
//
//[maxvertexcount(18)]
//void PointLightShadow_GS(triangle VertexShaderOutput input[3], inout TriangleStream<GeometeryShaderOutput> OutputStream)
//{
//    GeometeryShaderOutput OUT;
//    
//    for (int i = 0; i < 6; i++)
//    {
//        OUT.TargetIndex = i;
//        OUT.Position = input[0].Position;
//        OutputStream.Append(OUT);
//        
//        OUT.Position = input[1].Position;
//        OutputStream.Append(OUT);
//
//        OUT.Position = input[2].Position;
//        OutputStream.Append(OUT);
//        
//        OutputStream.RestartStrip();
//    }
//
//    
//}