
struct Resources
{
    uint ViewBufferIndex;
    uint DeferredColorIndex;
    uint StaticSamplerBufferIndex;
};

ConstantBuffer<Resources> BindlessResources : register(b0);

struct ViewBuffer
{
    matrix WorldToView;
    matrix ViewToProjection;
    matrix WorldToProjection;
    matrix ProjectionToView;
    matrix ProjectionToWorld;
    matrix LocalToCameraView;

    uint2 RenderSize;
};

struct StaticSamplers
{
    uint LinearSamplerIndex;
};

struct VertexInputPosUV
{
    float3 Position : POSITION;
    float2 UV : TEXCOORD;
};

struct VertexShaderOutput
{
    float2 UV : TEXCOORD0;
    float4 Position : SV_Position;
};

VertexShaderOutput Main_VS(VertexInputPosUV IN)
{
    VertexShaderOutput OUT;

    ConstantBuffer<ViewBuffer> View = ResourceDescriptorHeap[BindlessResources.ViewBufferIndex];
    
    OUT.Position = mul(View.LocalToCameraView, float4(IN.Position, 1.0f));
    OUT.Position.z = 0;
    OUT.UV = IN.UV;

    return OUT;
}

struct PixelShaderInput
{
    float2 UV : TEXCOORD0;
};

float3 ACESFilmic( float3 x, float A, float B, float C, float D, float E, float F )
{
    return ( ( x * ( A * x + C * B ) + D * E ) / ( x * ( A * x + B ) + D * F ) ) - ( E / F );
}

float4 Main_PS(PixelShaderInput IN) : SV_Target
{
    ConstantBuffer<StaticSamplers> StaticSamplers = ResourceDescriptorHeap[BindlessResources.StaticSamplerBufferIndex];
    
    Texture2D HdrImage = ResourceDescriptorHeap[BindlessResources.DeferredColorIndex];
    SamplerState LinearSampler = ResourceDescriptorHeap[StaticSamplers.LinearSamplerIndex];
    
    //float Exposure = 8.0f;
    //float Gamma = 2.2f;
    //
    //float3 HdrColor = HdrImage.Sample(LinearSampler, IN.UV).xyz;
    //float3 Mapped = float3(1.0f, 1.0f, 1.0f) - exp(-HdrColor * Exposure.xxx);
    //
    //float a = 1.0f / Gamma;
    //Mapped = pow(Mapped, float3(a.xxx));
    //
    //return float4(Mapped, 1);
    
    float A = 0.22f;
    float B = 0.30f;
    float C = 0.10f;
    float D = 0.20f;
    float E = 0.01f;
    float F = 0.30f;
    float LinearWhite = 11.02f;
    
    float Exposure = 2.0f;
    float Gamma = 2.2f;
    
    float3 HdrColor = HdrImage.Sample(LinearSampler, IN.UV).xyz;
    HdrColor *= exp2(Exposure);
    
    float3 SDR = ACESFilmic(HdrColor, A, B, C, D, E, F) /
              ACESFilmic(LinearWhite, A, B, C, D, E, F);
    
    return float4(pow(abs(SDR), 1.0f / Gamma), 1);
}