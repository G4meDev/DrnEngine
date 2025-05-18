struct VertexInputPosUV
{
    float3 Position : POSITION;
    float2 UV : TEXCOORD;
};

struct ViewBuffer
{
    matrix LocalToView;
    uint2 RenderTargetSize;
};

ConstantBuffer<ViewBuffer> View : register(b0);

// if want to use depth should use another srv as float
Texture2D<uint2> StencilTexture : register(t0);
SamplerState TextureSampler : register(s0);

struct VertexShaderOutput
{
    float2 UV : TEXCOORD0;
    float4 Position : SV_Position;
};

VertexShaderOutput Main_VS(VertexInputPosUV IN)
{
    VertexShaderOutput OUT;

    OUT.Position = mul(View.LocalToView, float4(IN.Position, 1.0f));
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
    //float RawStencil = StencilTexture.Sample(TextureSampler, IN.UV).x;
    //return float4( RawStencil.xxx * 100, 1.0f );

    float3 SelectedColor = float3(0.95f, 0.65, 0.3f);
    //uint StepSize = 1;
    //uint StepCount = 2;

    uint2 ScreenPos = IN.UV * View.RenderTargetSize;
    //float Coverage = 0;

    //for (uint i = 1; i < StepCount + 1; i++)
    //{
    //    Coverage += StencilTexture.Load(float3(ScreenPos + uint2( 1, 0 ) * StepSize * i, 0)).y == 255;
    //    Coverage += StencilTexture.Load(float3(ScreenPos + uint2( 1, 1 ) * StepSize * i, 0)).y == 255;
    //    Coverage += StencilTexture.Load(float3(ScreenPos + uint2( 0, 1 ) * StepSize * i, 0)).y == 255;
    //    Coverage += StencilTexture.Load(float3(ScreenPos + uint2( -1, 1 ) * StepSize * i, 0)).y == 255;
    //    Coverage += StencilTexture.Load(float3(ScreenPos + uint2( -1, 0 ) * StepSize * i, 0)).y == 255;
    //    Coverage += StencilTexture.Load(float3(ScreenPos + uint2( -1, -1 ) * StepSize * i, 0)).y == 255;
    //    Coverage += StencilTexture.Load(float3(ScreenPos + uint2( 0, -1 ) * StepSize * i, 0)).y == 255;
    //    Coverage += StencilTexture.Load(float3(ScreenPos + uint2( 1, -1 ) * StepSize * i, 0)).y == 255;
    //}
    //
    //Coverage /= 8 * StepCount;
    
    //Coverage = (Coverage < 0.6f && Coverage > 0.01f) ? 1 : 0;
    
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