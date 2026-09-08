#include "Common.hlsl"

// DOMAIN_SURFACE
// BLEND_OPAQUE
// SHADING_LIT

// SUPPORT_STATICMESH
// SUPPORT_SKELETALMESH

// SUPPORT_MAIN_PASS
// SUPPORT_HIT_PROXY_PASS
// SUPPORT_PRE_PASS
// SUPPORT_EDITOR_SELECTION_PASS

ConstantBuffer<StandardResources> BindlessResources : register(b0);

struct VertexShaderOutput
{
    float4 Position : SV_Position;
    float3 Normal : Normal;
    
#if HITPROXY_PASS
    uint BoneIndex : ID;
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
#endif
};

VertexShaderOutput Main_VS(VertexInput IN)
{
    VertexShaderOutput OUT;
    
    ConstantBuffer<ViewBuffer> View = ResourceDescriptorHeap[BindlessResources.ViewIndex];
    
    float3 LocalPosition;
    float3 LocalNormal;
    
#if STATICMESH
    ConstantBuffer<PrimitiveBuffer> Primitive = ResourceDescriptorHeap[BindlessResources.PrimitiveIndex];
    LocalPosition = IN.Position;
    LocalNormal = IN.Normal;
    
#elif SKELETALMESH
    ConstantBuffer<SkeletalMeshPrimitiveBuffer> Primitive = ResourceDescriptorHeap[BindlessResources.PrimitiveIndex];
    ConstantBuffer<SkeletalMeshBoneData> Bones = ResourceDescriptorHeap[Primitive.BoneMatricesIndex];
    BoneBlendVertexData BlendedData = BoneBlendVertex(IN, Bones);
    
    LocalPosition = BlendedData.Position;
    LocalNormal = BlendedData.Normal;
#endif
    
    matrix LocalToWorld = Primitive.LocalToWorld;
    float4 WorldPosition = mul(LocalToWorld, float4(LocalPosition, 1.0f));
    OUT.Position = mul(View.WorldToProjection, WorldPosition);
    OUT.Normal = normalize(mul((float3x3) LocalToWorld, LocalNormal));
    
    
#if HITPROXY_PASS && SKELETALMESH
    OUT.BoneIndex = 0;

#if SKELETALMESH
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
#endif
    
    return OUT;
}

// -------------------------------------------------------------------------------------

struct PixelShaderInput
{
    float4 Position : SV_Position;
    float3 Normal : Normal;
    
#if HITPROXY_PASS
    uint BoneIndex : ID;
#endif
};

PixelShaderOutput Main_PS(PixelShaderInput IN) : SV_Target
{
#if STATICMESH
    ConstantBuffer<PrimitiveBuffer> Primitive = ResourceDescriptorHeap[BindlessResources.PrimitiveIndex];
#elif SKELETALMESH
    ConstantBuffer<SkeletalMeshPrimitiveBuffer> Primitive = ResourceDescriptorHeap[BindlessResources.PrimitiveIndex];
#endif

    PixelShaderOutput OUT;
    
#if MAIN_PASS
    OUT.ColorDeferred = float4(0.0f, 0.0f, 0.0f, 1.0f);
    OUT.BaseColor = float4(0.7, 0.5, 1, 1);
    OUT.WorldNormal = EncodeNormal(IN.Normal);
    OUT.Masks = float4(0.2, 1, 0.4, Uint8ToFloat(SHADING_MODEL_LIT));
#elif HITPROXY_PASS
    OUT.Guid = Primitive.Guid;
#if SKELETALMESH
#endif
    OUT.Guid[2] = IN.BoneIndex;
#endif
    
    return OUT;
}