#include "Common.hlsl"

// DOMAIN_SURFACE
// BLEND_OPAQUE
// SHADING_LIT

// SUPPORT_STATICMESH

// SUPPORT_MAIN_PASS
// SUPPORT_PRE_PASS
// SUPPORT_HIT_PROXY_PASS
// SUPPORT_EDITOR_SELECTION_PASS
// SUPPORT_SHADOW_PASS

ConstantBuffer<StandardResources> BindlessResources : register(b0);

struct Primitive
{
    matrix LocalToWorld;
    matrix LocalToProjection;
    uint4 Guid;
    matrix PrevLocalToWorld;
    matrix PrevLocalToProjection;
};

struct ParametersBuffers
{
    VECTOR(TintColor, TintColor)
    
    SCALAR(RoughnessIntensity, RoughnessIntensity)
    SCALAR(NormalIntensity, NormalIntensity)
    SCALAR(EmssiveIntensity, EmssiveIntensity)
    SCALAR(TintColorIntensity, TintColorIntensity)

    TEX2D(BaseColor, BaseColorTexture)
    TEX2D(Normal, NormalTexture)
    TEX2D(Masks, MasksTexture)
    TEX2D(Emssive, EmssiveTexture)
};

struct VertexShaderOutput
{
    float4 Color : COLOR;
    float3 Normal : NORMAL;
    float3x3 TBN : TBN;
    float2 UV : TEXCOORD;
    float4 ScreenPos : TEXCOORD1;
    float4 PrevScreenPos : TEXCOORD2;
    float4 Position : SV_Position;
};

VertexShaderOutput Main_VS(VertexInputStaticMesh IN)
{
    VertexShaderOutput OUT;
    
    ConstantBuffer<ViewBuffer> View = ResourceDescriptorHeap[BindlessResources.ViewIndex];
    ConstantBuffer<PrimitiveBuffer> Primitive = ResourceDescriptorHeap[BindlessResources.PrimitiveIndex];

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
    OUT.UV = IN.UV1;
    
    OUT.ScreenPos = OUT.Position;
    //OUT.PrevScreenPos = mul(Primitive.PrevLocalToProjection, float4(IN.Position, 1.0f));
    OUT.PrevScreenPos = OUT.Position;

#endif
    
    return OUT;
}

//// -------------------------------------------------------------------------------------

struct PixelShaderInput
{
#if SHADOW_PASS
    float4 Position : SV_Position;
#else
    float4 Color : COLOR;
    float3 Normal : NORMAL;
    float3x3 TBN : TBN;
    float2 UV : TEXCOORD;
    float4 ScreenPos : TEXCOORD1;
    float4 PrevScreenPos : TEXCOORD2;
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

//#define MAIN_PASS 1

PixelShaderOutput Main_PS(PixelShaderInput IN) : SV_Target
{
    PixelShaderOutput OUT;
 
#if MAIN_PASS

    ConstantBuffer<ParametersBuffers> Parameters = ResourceDescriptorHeap[BindlessResources.ParametersBufferIndex];

    Texture2D BaseColorTexture = ResourceDescriptorHeap[Parameters.BaseColor_Texture];
    SamplerState BaseColorSampler = ResourceDescriptorHeap[Parameters.BaseColor_Sampler];
    
    Texture2D NormalTexture = ResourceDescriptorHeap[Parameters.Normal_Texture];
    SamplerState NormalSampler = ResourceDescriptorHeap[Parameters.Normal_Sampler];
    
    Texture2D MasksTexture = ResourceDescriptorHeap[Parameters.Masks_Texture];
    SamplerState MasksSampler = ResourceDescriptorHeap[Parameters.Masks_Sampler];
    
    Texture2D EmssiveTexture = ResourceDescriptorHeap[Parameters.Emssive_Texture];
    SamplerState EmssiveSampler = ResourceDescriptorHeap[Parameters.Emssive_Sampler];
    
    float3 BaseColor = BaseColorTexture.Sample(BaseColorSampler, IN.UV).xyz;
    float3 Masks = MasksTexture.Sample(MasksSampler, IN.UV).xyz;
    float3 Emssive = EmssiveTexture.Sample(EmssiveSampler, IN.UV).xyz;

    //float3 Normal = NormalTexture.Sample(LinearSampler, IN.UV).rbg;
    float3 Normal = NormalTexture.Sample(NormalSampler, IN.UV).rgb;
    Masks.g *= Parameters.RoughnessIntensity;
    //Masks.g = Parameters.RoughnessIntensity;

    Normal = ReconstructTextureNormal(Normal.xy);
    Normal = lerp( float3(0.0f, 1.0f, 0.0f), Normal, Parameters.NormalIntensity );
    Normal = normalize(mul(Normal, IN.TBN));
    //Normal = EncodeNormal(Normal);
    float2 N = EncodeNormal(Normal);
    
    
    OUT.ColorDeferred = float4(Emssive * Parameters.EmssiveIntensity, 1);
    OUT.BaseColor = float4(BaseColor, 1);
    OUT.BaseColor = lerp(OUT.BaseColor, Parameters.TintColor, Parameters.TintColorIntensity);
    //OUT.WorldNormal = float4( Normal, 0);
    OUT.WorldNormal = N;
    OUT.Masks = float4(Masks, 1.0f/255);
    
    //float2 Velocity = IN.ScreenPos.xy / IN.ScreenPos.w - IN.PrevScreenPos.xy / IN.PrevScreenPos.w;
    //Velocity = Velocity * 0.25f + 0.5f;
    //OUT.Velocity = Velocity;
    
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