
#define GROUP_TILE_SIZE 8

struct Resources
{
    uint ViewBufferIndex;
    uint unused;
    uint StaticSamplerBufferIndex;
    int NumMips;
	
    int MipIndex;
    int FaceThreadGroupSize;
    int2 ValidDispatchCoord;
    
    uint SourceCubemapTexture;
    uint OutTextureMip;
    uint2 Pad;
	
    float4 LowerHemisphereSolidColor;
};

ConstantBuffer<Resources> BindlessResources : register(b0);

struct StaticSamplers
{
    uint LinearSamplerIndex;
    uint PointSamplerIndex;
    uint LinearCmpSamplerIndex;
    uint LinearClampIndex;
    uint PointClampIndex;
};

float3 GetCubemapVector(float2 ScaledUVs, int InCubeFace)
{
	float3 CubeCoordinates;

	if (InCubeFace == 0)
	{
		CubeCoordinates = float3(1, -ScaledUVs.y, -ScaledUVs.x);
	}
	else if (InCubeFace == 1)
	{
		CubeCoordinates = float3(-1, -ScaledUVs.y, ScaledUVs.x);
	}
	else if (InCubeFace == 2)
	{
		CubeCoordinates = float3(ScaledUVs.x, 1, ScaledUVs.y);
	}
	else if (InCubeFace == 3)
	{
		CubeCoordinates = float3(ScaledUVs.x, -1, -ScaledUVs.y);
	}
	else if (InCubeFace == 4)
	{
		CubeCoordinates = float3(ScaledUVs.x, -ScaledUVs.y, 1);
	}
	else
	{
		CubeCoordinates = float3(-ScaledUVs.x, -ScaledUVs.y, -1);
	}

	return CubeCoordinates;
}

[numthreads(GROUP_TILE_SIZE, GROUP_TILE_SIZE, 1)]
void Main_CS(uint3 ThreadId : SV_DispatchThreadID)
{
    ConstantBuffer<StaticSamplers> StaticSamplers = ResourceDescriptorHeap[BindlessResources.StaticSamplerBufferIndex];
    SamplerState PointSampler = ResourceDescriptorHeap[StaticSamplers.PointClampIndex];
    
    TextureCube SourceCubemapTexture = ResourceDescriptorHeap[BindlessResources.SourceCubemapTexture];
    RWTexture2DArray<float4> OutTexture = ResourceDescriptorHeap[BindlessResources.OutTextureMip];

    
    const uint2 FaceCoord = uint2(ThreadId.x % uint(BindlessResources.FaceThreadGroupSize), ThreadId.y);
	if (any(FaceCoord >= uint2(BindlessResources.ValidDispatchCoord)))
	{
		return;
	}
	const int SelectedCubeFace = int(ThreadId.x) / BindlessResources.FaceThreadGroupSize;
	float2 ScaledUVs = ((float2(FaceCoord) + 0.5f) / float2(BindlessResources.ValidDispatchCoord)) * 2.0f - 1.0f;
	float4 OutColor;
    
    float3 CubeCoordinates = GetCubemapVector(ScaledUVs, SelectedCubeFace);

	float3 N = normalize(CubeCoordinates);
	if (N.y < 0.0f)
	{
        OutTexture[uint3(FaceCoord, SelectedCubeFace)] = float4(BindlessResources.LowerHemisphereSolidColor.rgb * BindlessResources.LowerHemisphereSolidColor.a, 1.0f);
    }
}