
struct Resources
{
    uint ViewBufferIndex;
    uint TextureIndex;
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

// if want to use depth should use another srv as float
//Texture2D<uint2> StencilTexture : register(t0);
//SamplerState TextureSampler : register(s0);

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

float4 Main_PS(PixelShaderInput IN) : SV_Target
{
    ConstantBuffer<ViewBuffer> View = ResourceDescriptorHeap[BindlessResources.ViewBufferIndex];
    Texture2D<uint2> StencilTexture = ResourceDescriptorHeap[BindlessResources.TextureIndex];
      
    float3 SelectedColor = float3(0.95f, 0.65, 0.3f);
    uint2 ScreenPos = IN.UV * View.RenderSize;
    uint Coverage = 0;
   
    Coverage += StencilTexture.Load(float3(ScreenPos, 0)).y == 255;
    Coverage += StencilTexture.Load(float3(ScreenPos + uint2( 1, 0 ) * 2, 0)).y == 255;
    Coverage += StencilTexture.Load(float3(ScreenPos + uint2( 1, 1 ) * 1, 0)).y == 255;
    Coverage += StencilTexture.Load(float3(ScreenPos + uint2( 0, 1 ) * 2, 0)).y == 255;
    Coverage += StencilTexture.Load(float3(ScreenPos + uint2( -1, 1 ) * 1, 0)).y == 255;
    Coverage += StencilTexture.Load(float3(ScreenPos + uint2( -1, 0 ) * 2, 0)).y == 255;
    Coverage += StencilTexture.Load(float3(ScreenPos + uint2( -1, -1 ) * 1, 0)).y == 255;
    Coverage += StencilTexture.Load(float3(ScreenPos + uint2( 0, -1 ) * 2, 0)).y == 255;
    Coverage += StencilTexture.Load(float3(ScreenPos + uint2( 1, -1 ) * 1, 0)).y == 255;
    
    float Alpha = Coverage > 1 && Coverage < 9;
    
    return float4( SelectedColor , Alpha);
}