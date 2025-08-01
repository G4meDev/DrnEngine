//#include "../../../Engine/Content/Materials/Common.hlsl"

// SUPPORT_MAIN_PASS
// SUPPORT_HIT_PROXY_PASS
// SUPPORT_EDITOR_SELECTION_PASS
// SUPPORT_SHADOW_PASS

float3 EncodeNormal(float3 Normal)
{
    return Normal * 0.5 + 0.5;
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

struct TextureBuffers
{
    uint BaseColorTexture; // @TEX2D BaseColorTexture
    uint NormalTexture; // @TEX2D NormalTexture
    uint MasksTexture; // @TEX2D MasksTexture
};

struct ScalarBuffer
{
    float TintIntensity; // @SCALAR ColorIntensity
};

struct VectorBuffer
{
    float4 TintColor; // @VECTOR TintColor
};

struct ShadowDepth
{
    matrix WorldToProjectionMatrices[6];
    float3 LightPos;
    float NearZ;
    float Radius;
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
    float4 Color : COLOR;
    float3 Normal : NORMAL;
    float3x3 TBN : TBN;
    float2 UV : TEXCOORD;
    float4 Position : SV_Position;
};

VertexShaderOutput Main_VS(VertexInputStaticMesh IN)
{
    VertexShaderOutput OUT;

#if SHADOW_PASS
    ConstantBuffer<Primitive> P = ResourceDescriptorHeap[BindlessResources.PrimitiveIndex];
    OUT.Position = mul(P.LocalToWorld, float4(IN.Position, 1.0f));
    
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
    float4 WorldNormal : SV_TARGET2;
    float4 Masks : SV_TARGET3;
#elif HITPROXY_PASS
    uint4 Guid;
#elif EDITOR_PRIMITIVE_PASS
    float4 Color;
#elif SHADOW_PASS
    float Depth : SV_Depth;
#endif
};

PixelShaderOutput Main_PS(PixelShaderInput IN) : SV_Target
{
    PixelShaderOutput OUT;
 
#if MAIN_PASS

    ConstantBuffer<StaticSamplers> StaticSamplers = ResourceDescriptorHeap[BindlessResources.StaticSamplerBufferIndex];
    SamplerState LinearSampler = ResourceDescriptorHeap[StaticSamplers.LinearSamplerIndex];
    
    ConstantBuffer<TextureBuffers> Textures = ResourceDescriptorHeap[BindlessResources.TextureBufferIndex];
    Texture2D BaseColorTexture = ResourceDescriptorHeap[Textures.BaseColorTexture];
    Texture2D NormalTexture = ResourceDescriptorHeap[Textures.NormalTexture];
    Texture2D MasksTexture = ResourceDescriptorHeap[Textures.MasksTexture];
    
    float3 BaseColor = BaseColorTexture.Sample(LinearSampler, IN.UV).xyz;
    float3 Masks = MasksTexture.Sample(LinearSampler, IN.UV).xyz;

    ConstantBuffer<ScalarBuffer> Scalars = ResourceDescriptorHeap[BindlessResources.ScalarBufferIndex];
    ConstantBuffer<VectorBuffer> Vectors = ResourceDescriptorHeap[BindlessResources.VectorBufferIndex];

    float3 Normal = NormalTexture.Sample(LinearSampler, IN.UV).rbg;
    Normal.z = 1 - Normal.z;
    Normal = Normal * 2 - 1;
    Normal = normalize(mul(Normal, IN.TBN));
    Normal = EncodeNormal(Normal);
    
    OUT.ColorDeferred = float4(0.0, 0.0, 0.0, 1);
    OUT.BaseColor = float4(BaseColor, 1);
    OUT.BaseColor = lerp(OUT.BaseColor, Vectors.TintColor, Scalars.TintIntensity);
    OUT.WorldNormal = float4( Normal, 0);
    OUT.Masks = float4(Masks, 1);
    
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
            //OUT.Position = mul(ShadowDepthBuffer.WorldToProjectionMatrices[CubeFaceIndex], float4(WorldPosition, 1.0f));
            OUT.Position = mul(ShadowDepthBuffer.WorldToProjectionMatrices[CubeFaceIndex], float4(WorldPosition, 1.0f));
            OUT.Position.z /= OUT.Position.w;
            //float d = distance(ShadowDepthBuffer.LightPos, WorldPosition);
            //OUT.Position.z = d/ShadowDepthBuffer.Radius * OUT.Position.w;
            OutputStream.Append(OUT);
        }
		OutputStream.RestartStrip();
    }
}

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