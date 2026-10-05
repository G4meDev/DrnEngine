#include "Common.hlsl"

// DOMAIN_SURFACE
// BLEND_OPAQUE
// SHADING_UNLIT

// SUPPORT_SKELETALMESH

// SUPPORT_MAIN_PASS
// SUPPORT_PRE_PASS
// SUPPORT_HIT_PROXY_PASS
// SUPPORT_EDITOR_SELECTION_PASS

ConstantBuffer<StandardResources> BindlessResources : register(b0);

struct ParametersBuffers
{
    SCALAR(BoneIndex, BoneIndex)
};

//#define MAIN_PASS 1
//#define HITPROXY_PASS 1

struct VertexShaderOutput
{
    float4 Position : SV_Position;
#if MAIN_PASS
    float3 VertexColor : COLOR;
#endif
    
#if HITPROXY_PASS
    uint BoneIndex : ID;
#endif
};

VertexShaderOutput Main_VS(
    VertexInput IN
)
{
    VertexShaderOutput OUT;

    ConstantBuffer<ViewBuffer> View = ResourceDescriptorHeap[BindlessResources.ViewIndex];
    ConstantBuffer<SkeletalMeshPrimitiveBuffer> Primitive = ResourceDescriptorHeap[BindlessResources.PrimitiveIndex];
    ConstantBuffer<ParametersBuffers> Parameters = ResourceDescriptorHeap[BindlessResources.ParametersBufferIndex];
    
    ConstantBuffer<SkeletalMeshBoneData> Bones = ResourceDescriptorHeap[Primitive.BoneMatricesIndex];
    
    BoneBlendVertexData BlendedData = BoneBlendVertex(IN, Bones);
    
    matrix LocalToWorld = Primitive.LocalToWorld;
    float4 WorldPosition = mul(LocalToWorld, float4(BlendedData.Position, 1.0f));
    
    OUT.Position = mul(View.WorldToProjection, WorldPosition);
    
#if MAIN_PASS
    OUT.VertexColor = 0;
    
    [unroll]
    for (int i = 0; i < MAX_EFFECTIVE_BONES; i++)
    {
        if (IN.BoneIndices[i] == Parameters.BoneIndex)
        {
            OUT.VertexColor = IN.BoneWeights[i].xxx;
        }
    }
#endif
    
#if HITPROXY_PASS
    float MaxBoneWeight = 0.0f;
    [unroll]
    for (int i = 0; i < MAX_EFFECTIVE_BONES; i++)
    {
        const float BoneWeight = IN.BoneWeights[i];
        if (BoneWeight > MaxBoneWeight)
        {
            MaxBoneWeight = BoneWeight;
            OUT.BoneIndex = IN.BoneIndices[i];
        }
    }
#endif
    
    return OUT;
}

//// -------------------------------------------------------------------------------------

struct PixelShaderInput
{
    float4 Position : SV_Position;
#if MAIN_PASS
    float3 VertexColor : COLOR;
#endif
    
#if HITPROXY_PASS
    uint BoneIndex : ID;
#endif
};

struct PixelShaderOutput
{
#if MAIN_PASS
    float4 ColorDeferred : SV_TARGET0;
    float4 Masks : SV_TARGET3;
#elif HITPROXY_PASS
    uint4 Guid : SV_TARGET0;
#elif EDITOR_PRIMITIVE_PASS
    float4 Color : SV_TARGET0;
#elif SHADOW_PASS
#endif
};

PixelShaderOutput Main_PS(PixelShaderInput IN)
{
    PixelShaderOutput OUT;
 
#if MAIN_PASS

    ConstantBuffer<ParametersBuffers> Parameters = ResourceDescriptorHeap[BindlessResources.ParametersBufferIndex];

    ConstantBuffer<ViewBuffer> View = ResourceDescriptorHeap[BindlessResources.ViewIndex];
    ConstantBuffer<StaticSamplers> StaticSamplers = ResourceDescriptorHeap[BindlessResources.StaticSamplerBufferIndex];
    SamplerState LinearSampler = ResourceDescriptorHeap[StaticSamplers.LinearSamplerIndex];
    
    OUT.ColorDeferred = float4(IN.VertexColor, 1);
    OUT.Masks = float4(0, 0, 1, Uint8ToFloat(SHADING_MODEL_UNLIT));
    
#elif HITPROXY_PASS
    ConstantBuffer<PrimitiveBuffer> P = ResourceDescriptorHeap[BindlessResources.PrimitiveIndex];
    OUT.Guid = P.Guid;
    OUT.Guid[2] = IN.BoneIndex;
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