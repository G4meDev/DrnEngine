#include "Common.hlsl"

// DOMAIN_SURFACE

// SUPPORT_STATICMESH

// SUPPORT_MAIN_PASS
// SUPPORT_PRE_PASS
// SUPPORT_HIT_PROXY_PASS
// SUPPORT_EDITOR_SELECTION_PASS
// SUPPORT_SHADOW_PASS

ConstantBuffer<StandardResources> BindlessResources : register(b0);

struct ParametersBuffers
{
    VECTOR(TintColor, TintColor)
    
    SCALAR(GrassUVScale, GrassUVScale)
    SCALAR(RoughnessIntensity, RoughnessIntensity)
    SCALAR(NormalIntensity, NormalIntensity)
    SCALAR(FloorAlpha, FloorAlpha)
    SCALAR(GrassAlpha, GrassAlpha)
    SCALAR(Depth, Depth)

    TEX2D(FloorAlbedo, FloorAlbedo)
    TEX2D(FloorNormal, FloorNormal)
    TEX2D(FloorMasks, FloorMasks)
    
    TEX2D(GrassAlbedo, GrassAlbedo)
    TEX2D(GrassNormal, GrassNormal)
    TEX2D(GrassMasks, GrassMasks)
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

    ConstantBuffer<ViewBuffer> View = ResourceDescriptorHeap[BindlessResources.ViewIndex];
    ConstantBuffer<PrimitiveBuffer> Primitive = ResourceDescriptorHeap[BindlessResources.PrimitiveIndex];
    ConstantBuffer<ParametersBuffers> Parameters = ResourceDescriptorHeap[BindlessResources.ParametersBufferIndex];

    matrix LocalToWorld;
#if STATICMESH
    LocalToWorld = Primitive.LocalToWorld;
#elif INSTANCED
    LocalToWorld = GetLocalToWorld(IN);
#endif
    
    float4 WorldPosition = mul(LocalToWorld, float4(IN.Position, 1.0f));
    
#if SHADOW_PASS_POINTLIGHT
    OUT.Position = WorldPosition;
    
    OUT.TBN = float3x3(IN.Position,IN.Position,IN.Position);
    OUT.Color = float4(IN.Color, 1.0f);
    OUT.Normal = IN.Position;
    OUT.UV = IN.UV1;
#elif SHADOW_PASS_SPOTLIGHT
    ConstantBuffer<ShadowDepth> ShadowBuffer = ResourceDescriptorHeap[BindlessResources.ShadowDepthBuffer];
    OUT.Position = mul(ShadowBuffer.WorldToProjectionMatrix, WorldPosition);

    OUT.TBN = float3x3(IN.Position,IN.Position,IN.Position);
    OUT.Color = float4(IN.Color, 1.0f);
    OUT.Normal = IN.Position;
    OUT.UV = IN.UV1;
#else
        
    float3 WorldNormal = normalize(mul((float3x3)LocalToWorld, IN.Normal));
    float3 WorldTangent = normalize(mul((float3x3)LocalToWorld, IN.Tangent));
    OUT.TBN = GetTBN(WorldNormal, WorldTangent);
    
    OUT.Position = mul(View.WorldToProjection, WorldPosition);
    OUT.Color = float4(IN.Color, 1.0f);
    OUT.Normal = WorldNormal;
    //OUT.UV = IN.UV1;
    OUT.UV = WorldPosition.xz * Parameters.GrassUVScale;
    
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
    float4 MasksB : SV_TARGET4;
#elif HITPROXY_PASS
    uint4 Guid;
#elif EDITOR_PRIMITIVE_PASS
    float4 Color;
#elif SHADOW_PASS
#endif
};

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

    ConstantBuffer<ParametersBuffers> Parameters = ResourceDescriptorHeap[BindlessResources.ParametersBufferIndex];

    ConstantBuffer<ViewBuffer> View = ResourceDescriptorHeap[BindlessResources.ViewIndex];
    ConstantBuffer<StaticSamplers> StaticSamplers = ResourceDescriptorHeap[BindlessResources.StaticSamplerBufferIndex];
    SamplerState LinearSampler = ResourceDescriptorHeap[StaticSamplers.LinearSamplerIndex];
    
    Texture2D FloorAlbedoTexture = ResourceDescriptorHeap[Parameters.FloorAlbedo_Texture];
    SamplerState FloorAlbedoSampler = ResourceDescriptorHeap[Parameters.FloorAlbedo_Sampler];
    Texture2D FloorNormalTexture = ResourceDescriptorHeap[Parameters.FloorNormal_Texture];
    SamplerState FloorNormalSampler = ResourceDescriptorHeap[Parameters.FloorNormal_Sampler];
    Texture2D FloorMasksTexture = ResourceDescriptorHeap[Parameters.FloorMasks_Texture];
    SamplerState FloorMasksSampler = ResourceDescriptorHeap[Parameters.FloorMasks_Sampler];
    
    Texture2D GrassAlbedoTexture = ResourceDescriptorHeap[Parameters.GrassAlbedo_Texture];
    SamplerState GrassAlbedoSampler = ResourceDescriptorHeap[Parameters.GrassAlbedo_Sampler];
    Texture2D GrassNormalTexture = ResourceDescriptorHeap[Parameters.GrassNormal_Texture];
    SamplerState GrassNormalSampler = ResourceDescriptorHeap[Parameters.GrassNormal_Sampler];
    Texture2D GrassMasksTexture = ResourceDescriptorHeap[Parameters.GrassMasks_Texture];
    SamplerState GrassMasksSampler = ResourceDescriptorHeap[Parameters.GrassMasks_Sampler];

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
    BlendLayer = LayerHeightBlend(FloorLayer, FloorMasks.r, 1 - VertexColor.r , GrassLayer, GrassMasks.r, VertexColor.r, Parameters.Depth);
    
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
    Masks.g *= Parameters.RoughnessIntensity;

    Normal = ReconstructTextureNormal(Normal.xy);
    Normal = lerp( float3(0.0f, 1.0f, 0.0f), Normal, Parameters.NormalIntensity );
    Normal = normalize(mul(Normal, IN.TBN));
    
    Normal = lerp(Normal, float3(0, 1, 0), ClampRange(WetnessMask, 0.45, 0.95));
    
    //float DitherNoise = InterleavedGradientNoise(IN.Position.xy, View.FrameIndexMod8);
    
    BaseColor = lerp(BaseColor, Parameters.TintColor.rgb, Parameters.TintColor.a);
    
    OUT.ColorDeferred = float4(0.0, 0.0, 0.0, 1);
    OUT.BaseColor = float4(BaseColor, 1);
    //OUT.BaseColor = lerp(OUT.BaseColor, Vectors.TintColor, Scalars.TintIntensity);
    OUT.WorldNormal = EncodeNormal(Normal);
    OUT.Masks = float4(Masks, 1.0f/255);
    
#elif HITPROXY_PASS
    ConstantBuffer<PrimitiveBuffer> P = ResourceDescriptorHeap[BindlessResources.PrimitiveIndex];
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