
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

	uint MipSize = (uint)1 << ( BindlessResources.NumMips - BindlessResources.MipIndex - 1 );

	float3 TangentZ = normalize( CubeCoordinates );
	float3 TangentX = normalize( cross( GetCubemapVector( ScaledUVs + float2(0,1), SelectedCubeFace), TangentZ ) );
	float3 TangentY = cross( TangentZ, TangentX );

	const float SampleOffset = 2.0 * 2 / MipSize;
	//const float SampleOffset = 1.0f / MipSize;

	float2 Offsets[] =
	{
		float2(-1, -1) * 0.7,
		float2( 1, -1) * 0.7,
		float2(-1,  1) * 0.7,
		float2( 1,  1) * 0.7,
		
		float2( 0, -1),
		float2(-1,  0),
		float2( 1,  0),
		float2( 0,  1),
	};

    //OutColor = SampleOffset;
    //OutColor = 0;
	
    OutColor = SourceCubemapTexture.SampleLevel(PointSampler, CubeCoordinates, 0);
	
	[unroll]
	for( uint i = 0; i < 8; i++ )
	{
		float Weight = 0.375;
	
		float3 SampleDir = CubeCoordinates;
		SampleDir += TangentX * ( Offsets[i].x * SampleOffset );
		SampleDir += TangentY * ( Offsets[i].y * SampleOffset );
        OutColor += SourceCubemapTexture.SampleLevel(PointSampler, SampleDir, 0) * Weight;
    }
	
	OutColor *= rcp( 1.0 + 1.0 + 2.0 );
	
	OutTexture[uint3(FaceCoord, SelectedCubeFace)] = OutColor;
}