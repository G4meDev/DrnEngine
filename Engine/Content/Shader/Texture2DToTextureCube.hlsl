//#include "../Materials/Common.hlsl"

struct Resources
{
    uint ViewBufferIndex;
};

ConstantBuffer<Resources> BindlessResources : register(b0);

struct MainData
{
    matrix WorldToProjectionMatrices[6];

    uint TextureIndex;
    uint LinearSamplerIndex;
};


struct VertexInput
{
    float3 Position : POSITION;
    float2 UV : TEXCOORD;
};

struct VertexShaderOutput
{
	float4 Position : SV_Position;
};

VertexShaderOutput Main_VS(VertexInput IN)
{
	VertexShaderOutput OUT;
	OUT.Position = float4(IN.Position, 1);
	return OUT;
}

// -------------------------------------------------------------------------------------

struct GeometeryShaderOutput
{
	float3 Direction : COLOR;
	float4 Position : SV_Position;
    uint TargetIndex : SV_RenderTargetArrayIndex;
};

[maxvertexcount(3)]
void Main_GS(triangle VertexShaderOutput input[3], inout TriangleStream<GeometeryShaderOutput> OutputStream)
{
    ConstantBuffer<MainData> MainBuffer = ResourceDescriptorHeap[BindlessResources.ViewBufferIndex];
    
	GeometeryShaderOutput V1;
	GeometeryShaderOutput V2;
	GeometeryShaderOutput V3;
    
    V1.Direction = normalize(input[0].Position.xyz);
    V2.Direction = normalize(input[1].Position.xyz);
    V3.Direction = normalize(input[2].Position.xyz);
   
    float3 AvgDir = V1.Direction + V2.Direction + V3.Direction;
    
    float3 AbsLightVector = abs(AvgDir);
    float MaxCoordinate = max(AbsLightVector.x, max(AbsLightVector.y, AbsLightVector.z));
    int CubeFaceIndex = 0;
    if (MaxCoordinate == AbsLightVector.x)
    {
        CubeFaceIndex = AbsLightVector.x == AvgDir.x ? 0 : 1;
    }
    else if (MaxCoordinate == AbsLightVector.y)
    {
        CubeFaceIndex = AbsLightVector.y == AvgDir.y ? 2 : 3;
    }
    else
    {
        CubeFaceIndex = AbsLightVector.z == AvgDir.z ? 4 : 5;
    }

    V1.TargetIndex = V2.TargetIndex = V3.TargetIndex = CubeFaceIndex;
    V1.Position = mul(MainBuffer.WorldToProjectionMatrices[CubeFaceIndex], float4(input[0].Position.xyz, 1));
    V2.Position = mul(MainBuffer.WorldToProjectionMatrices[CubeFaceIndex], float4(input[1].Position.xyz, 1));
    V3.Position = mul(MainBuffer.WorldToProjectionMatrices[CubeFaceIndex], float4(input[2].Position.xyz, 1));

	OutputStream.Append(V1);
	OutputStream.Append(V2);
	OutputStream.Append(V3);
		
	OutputStream.RestartStrip();
}

// -------------------------------------------------------------------------------------

struct PixelShaderInput
{
    float3 Direction : COLOR;
    float4 Position : SV_Position;
    uint TargetIndex : SV_RenderTargetArrayIndex;
};

float4 Main_PS(PixelShaderInput IN) : SV_Target
{
    const float PI = 3.14159265;
    
    ConstantBuffer<MainData> MainBuffer = ResourceDescriptorHeap[BindlessResources.ViewBufferIndex];
    Texture2D Texture = ResourceDescriptorHeap[MainBuffer.TextureIndex];
    SamplerState LinearSampler = ResourceDescriptorHeap[MainBuffer.LinearSamplerIndex];

    float3 Dir = IN.Direction;
    
    //float Long = atan2(Dir.y, Dir.x);
    //float Lat = atan2(Dir.z, sqrt(Dir.x * Dir.x + Dir.y * Dir.y));
    float Long = atan2(Dir.z, Dir.x);
    float Lat = atan2(Dir.y, sqrt(Dir.x * Dir.x + Dir.z * Dir.z));
    float2 UV = float2(Long / (2.0f * PI), Lat / PI + 0.5f);
    UV.y *= -1;
    
    //float2 UV = float2((1 + atan2(IN.Direction.z, -IN.Direction.x) / 3.14159265) / 2, acos(IN.Direction.y) / 3.14159265);
    
    float4 Sample = Texture.Sample(LinearSampler, UV);
    return Sample;
}