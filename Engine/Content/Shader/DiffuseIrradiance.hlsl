#include "Common.hlsl"
#define GROUP_TILE_SIZE 8

struct Resources
{
    uint ViewBufferIndex;
    float SampleOffset;
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
	
    float3 normal = normalize(CubeCoordinates);
    float3 up = float3(0.0, 1.0, 0.0);
    float3 right = normalize(cross(up, normal));
    up = normalize(cross(normal, right));
	
    float3 irradiance = 0;
    int nrSamples = 0;
    for (float phi = 0.0; phi < 2.0 * PI; phi += BindlessResources.SampleOffset)
    {
        for (float theta = 0.0; theta < 0.5 * PI; theta += BindlessResources.SampleOffset)
        {
            float3 tangentSample = float3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            float3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * normal;

            irradiance += SourceCubemapTexture.SampleLevel(PointSampler, sampleVec, 0).rgb;
            nrSamples++;
        }
    }

    OutColor = float4(PI * irradiance * (1.0f / float(nrSamples)), 1);

	OutTexture[uint3(FaceCoord, SelectedCubeFace)] = OutColor;
}