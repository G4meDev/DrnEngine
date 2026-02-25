#include "Common.hlsl"
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
    //SamplerState PointSampler = ResourceDescriptorHeap[StaticSamplers.LinearClampIndex];
    
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
	float3x3 TangentToWorld = GetTangentBasis( N );
	
	float Roughness = ComputeReflectionCaptureRoughnessFromMip( BindlessResources.MipIndex, BindlessResources.NumMips - 1 );
    //float Roughness = 0.1f;
	
	//OutTexture[uint3(FaceCoord, SelectedCubeFace)] = Roughness;
    //return;
	
	if( Roughness < 0.01 )
	{
		OutColor = SourceCubemapTexture.SampleLevel(PointSampler, CubeCoordinates, 0 );
        OutColor = -min(-OutColor, 0);
		
		OutTexture[uint3(FaceCoord, SelectedCubeFace)] = OutColor;
		return;
	}
	
    uint CubeSize = (uint)1 << (BindlessResources.NumMips - 1);
	const float SolidAngleTexel = 4.0f * PI / float(6 * CubeSize * CubeSize) * 2.0f;

	//const uint NumSamples = Roughness < 0.1 ? 32 : 64;
	
	uint NumSamples = Roughness < 0.1 ? 32 : 64;
	NumSamples *= 512;

	float4 FilteredColor = 0;
	[branch]
	if( Roughness > 0.99 )
	{		
		[loop]
		for( uint i = 0; i < NumSamples; i++ )
		{
			float2 E = Hammersley( i, NumSamples, 0 );
	
			float3 L = CosineSampleHemisphere( E ).xyz;
	
			float NoL = L.y;
	
			float PDF = NoL / PI;
			float SolidAngleSample = 1.0 / ( NumSamples * PDF );
			float Mip = 0.5 * log2( SolidAngleSample / SolidAngleTexel );
	
			L = mul( L, TangentToWorld );
			FilteredColor += SourceCubemapTexture.SampleLevel(PointSampler, L, Mip );
		}
	
		OutColor = FilteredColor / NumSamples;
	}
	else
	{
		float Weight = 0;
	
		[loop]
		for( uint i = 0; i < NumSamples; i++ )
		{
			float2 E = Hammersley( i, NumSamples, 0 );
			E.y *= 0.995;
			
			float3 H = ImportanceSampleGGX( E, Pow4(Roughness) ).xyz;
			
			float3 L = 2 * H.y * H - float3(0,1,0);
	
			float NoL = L.y;
			float NoH = H.y;
	
			if( NoL > 0 )
			{
				float PDF = D_GGX( Pow4(Roughness), NoH ) * 0.25;
				float SolidAngleSample = 1.0 / ( NumSamples * PDF );
				float Mip = 0.5 * log2( SolidAngleSample / SolidAngleTexel );
	
				float ConeAngle = acos( 1 - SolidAngleSample / (2*PI) );
	
				L = mul( L, TangentToWorld );
				FilteredColor += SourceCubemapTexture.SampleLevel(PointSampler, L, Mip ) * NoL;
				Weight += NoL;
			}
		}
	
		OutColor = FilteredColor / Weight;
	}
	
    OutColor = -min(-OutColor, 0);
	OutTexture[uint3(FaceCoord, SelectedCubeFace)] = OutColor;
}