
#define MAX_MIP_BATCH_SIZE 4
#define GROUP_TILE_SIZE 8

struct Resources
{
    uint ViewBufferIndex;
    uint ParentTextureIndex;
    uint StaticSamplerBufferIndex;
    uint WriteTextureIndex_1;
    
    uint WriteTextureIndex_2;
    uint WriteTextureIndex_3;
    uint WriteTextureIndex_4;
    uint Pad_1;
    
    float4 DispatchIdToUV;
    float2 InvSize;
};

ConstantBuffer<Resources> BindlessResources : register(b0);

struct StaticSamplers
{
    uint LinearSamplerIndex;
    uint PointSamplerIndex;
};

float4 Gather4(Texture2D Texture, SamplerState TextureSampler, float2 BufferUV, float2 InvSize)
{
    //float2 UV = min(BufferUV + float2(-0.25f, -0.25f) * InvSize, InputViewportMaxBound - InvSize);
    float2 UV = BufferUV + float2(-0.25f, -0.25f) * InvSize;
    return Texture.GatherRed(TextureSampler, UV, 0);
}

uint SignedRightShift(uint x, const int bitshift)
{
    if (bitshift > 0)
    {
        return x << asuint(bitshift);
    }
    else if (bitshift < 0)
    {
        return x >> asuint(-bitshift);
    }
    return x;
}

uint2 InitialTilePixelPositionForReduction2x2(const uint TileSizeLog2, uint SharedArrayId)
{
    uint x = 0;
    uint y = 0;

    [unroll]
    for (uint i = 0; i < TileSizeLog2; i++)
    {
        const uint DestBitId = TileSizeLog2 - 1 - i;
        const uint DestBitMask = 1u << DestBitId;
        x |= DestBitMask & SignedRightShift(SharedArrayId, int(DestBitId) - int(i * 2 + 0));
        y |= DestBitMask & SignedRightShift(SharedArrayId, int(DestBitId) - int(i * 2 + 1));
    }

    return uint2(x, y);
}

void OutputMipLevel(uint MipLevel, uint2 OutputPixelPos, float FurthestDeviceZ, RWTexture2D<float> Output_2, RWTexture2D<float> Output_3, RWTexture2D<float> Output_4)
{
#if MIP_LEVEL_COUNT >= 2
	if (MipLevel == 1)
	{
		Output_2[OutputPixelPos] = FurthestDeviceZ;
	}
#endif
#if MIP_LEVEL_COUNT >= 3
	else if (MipLevel == 2)
	{
		Output_3[OutputPixelPos] = FurthestDeviceZ;
	}
#endif
#if MIP_LEVEL_COUNT >= 4
	else if (MipLevel == 3)
	{
		Output_4[OutputPixelPos] = FurthestDeviceZ;
	}		
#endif
}

groupshared float SharedFurthestDeviceZ[GROUP_TILE_SIZE * GROUP_TILE_SIZE];

[numthreads(GROUP_TILE_SIZE, GROUP_TILE_SIZE, 1)]
void Main_CS(uint2 GroupId : SV_GroupID, uint GroupThreadIndex : SV_GroupIndex)
{
    ConstantBuffer<StaticSamplers> StaticSamplers = ResourceDescriptorHeap[BindlessResources.StaticSamplerBufferIndex];
    SamplerState PointSampler = ResourceDescriptorHeap[StaticSamplers.PointSamplerIndex];
    
    Texture2D ParentTexture = ResourceDescriptorHeap[BindlessResources.ParentTextureIndex];
    RWTexture2D<float> Output_1 = ResourceDescriptorHeap[BindlessResources.WriteTextureIndex_1];
    RWTexture2D<float> Output_2 = ResourceDescriptorHeap[BindlessResources.WriteTextureIndex_2];
    RWTexture2D<float> Output_3 = ResourceDescriptorHeap[BindlessResources.WriteTextureIndex_3];
    RWTexture2D<float> Output_4 = ResourceDescriptorHeap[BindlessResources.WriteTextureIndex_4];

#if MIP_LEVEL_COUNT == 1
    uint2 GroupThreadId = uint2(GroupThreadIndex % GROUP_TILE_SIZE, GroupThreadIndex / GROUP_TILE_SIZE);
#else
    uint2 GroupThreadId = InitialTilePixelPositionForReduction2x2(MAX_MIP_BATCH_SIZE - 1, GroupThreadIndex);
#endif
    
    uint2 DispatchThreadId = GROUP_TILE_SIZE * GroupId + GroupThreadId;
    
    float2 BufferUV = (DispatchThreadId + 0.5f) * BindlessResources.InvSize;
    float4 DeviceZ = Gather4(ParentTexture, PointSampler, BufferUV, BindlessResources.InvSize);

    float FurthestDeviceZ = min(min(DeviceZ.x, DeviceZ.y), min(DeviceZ.z, DeviceZ.w));
    uint2 OutputPixelPos = DispatchThreadId;
    
    Output_1[OutputPixelPos] = FurthestDeviceZ;

#if MIP_LEVEL_COUNT == 1

#else
    SharedFurthestDeviceZ[GroupThreadIndex] = FurthestDeviceZ;
	
    [unroll]
    for (uint MipLevel = 1; MipLevel < MIP_LEVEL_COUNT; MipLevel++)
    {
        const uint TileSize = GROUP_TILE_SIZE / (1u << MipLevel);
        const uint ReduceBankSize = TileSize * TileSize;
			
        if (MipLevel == 1)
            GroupMemoryBarrierWithGroupSync();

        [branch]
        if (GroupThreadIndex < ReduceBankSize)
        {
            float4 ParentFurthestDeviceZ;
            float4 ParentClosestDeviceZ;
            ParentFurthestDeviceZ[0] = FurthestDeviceZ;

            [unroll]
            for (uint i = 1; i < 4; i++)
            {
                uint LDSIndex = GroupThreadIndex + i * ReduceBankSize;
                ParentFurthestDeviceZ[i] = SharedFurthestDeviceZ[LDSIndex];
            }
				
            FurthestDeviceZ = min(min(ParentFurthestDeviceZ.x, ParentFurthestDeviceZ.y), min(ParentFurthestDeviceZ.z, ParentFurthestDeviceZ.w));
	
            OutputPixelPos = OutputPixelPos >> 1;
            OutputMipLevel(MipLevel, OutputPixelPos, FurthestDeviceZ, Output_2, Output_3, Output_4);
				
            SharedFurthestDeviceZ[GroupThreadIndex] = FurthestDeviceZ;
        }
    }
#endif
}