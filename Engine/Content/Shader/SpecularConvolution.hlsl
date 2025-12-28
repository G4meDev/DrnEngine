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
	float3x3 TangentToWorld = GetTangentBasis( N );

	float Roughness = ComputeReflectionCaptureRoughnessFromMip( MipIndex, NumMips - 1 );

	if( Roughness < 0.01 )
	{
		OutColor = SourceCubemapTexture.SampleLevel(SourceCubemapSampler, CubeCoordinates, 0 );
	#ifdef USE_COMPUTE
		OutTextureMipColor[uint3(FaceCoord, SelectedCubeFace)] = OutColor;
	#endif
		return;
	}

	uint CubeSize = 1 << ( NumMips - 1 );
	const float SolidAngleTexel = 4.0f * PI / float(6 * CubeSize * CubeSize) * 2.0f;

	//const uint NumSamples = 1024;
	const uint NumSamples = Roughness < 0.1 ? 32 : 64;
	
	float4 FilteredColor = 0;
	BRANCH
	if( Roughness > 0.99 )
	{
		// Roughness=1, GGX is constant. Use cosine distribution instead

		LOOP
		for( uint i = 0; i < NumSamples; i++ )
		{
			float2 E = Hammersley( i, NumSamples, 0 );

			float3 L = CosineSampleHemisphere( E ).xyz;

			float NoL = L.z;

			float PDF = NoL / PI;
			float SolidAngleSample = 1.0 / ( NumSamples * PDF );
			float Mip = 0.5 * log2( SolidAngleSample / SolidAngleTexel );

			L = mul( L, TangentToWorld );
			FilteredColor += SourceCubemapTexture.SampleLevel(SourceCubemapSampler, L, Mip );
		}

		OutColor = FilteredColor / NumSamples;
	}
	else
	{
		float Weight = 0;

		LOOP
		for( uint i = 0; i < NumSamples; i++ )
		{
			float2 E = Hammersley( i, NumSamples, 0 );

			// 6x6 Offset rows. Forms uniform star pattern
			//uint2 Index = uint2( i % 6, i / 6 );
			//float2 E = ( Index + 0.5 ) / 5.8;
			//E.x = frac( E.x + (Index.y & 1) * (0.5 / 6.0) );

			E.y *= 0.995;

			float3 H = ImportanceSampleGGX( E, Pow4(Roughness) ).xyz;
			float3 L = 2 * H.z * H - float3(0,0,1);

			float NoL = L.z;
			float NoH = H.z;

			if( NoL > 0 )
			{
				//float TexelWeight = CubeTexelWeight( L );
				//float SolidAngleTexel = SolidAngleAvgTexel * TexelWeight;

				//float PDF = D_GGX( Pow4(Roughness), NoH ) * NoH / (4 * VoH);
				float PDF = D_GGX( Pow4(Roughness), NoH ) * 0.25;
				float SolidAngleSample = 1.0 / ( NumSamples * PDF );
				float Mip = 0.5 * log2( SolidAngleSample / SolidAngleTexel );

				float ConeAngle = acos( 1 - SolidAngleSample / (2*PI) );

				L = mul( L, TangentToWorld );
				FilteredColor += SourceCubemapTexture.SampleLevel(SourceCubemapSampler, L, Mip ) * NoL;
				Weight += NoL;
			}
		}

		OutColor = FilteredColor / Weight;
	}

	OutTextureMipColor[uint3(FaceCoord, SelectedCubeFace)] = OutColor;
}