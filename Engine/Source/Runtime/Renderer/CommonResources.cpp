#include "DrnPCH.h"
#include "CommonResources.h"

#include "Runtime/Renderer/RenderGeometeryHelper.h"

#define PAR_SHAPES_IMPLEMENTATION
#include "ThirdParty/par/par_shapes.h"

#include <dxcapi.h>

LOG_DEFINE_CATEGORY( LogCommonResources, "CommonResources" );

namespace Drn
{
	CommonResources* CommonResources::m_SingletonInstance = nullptr;

	D3D12_INPUT_ELEMENT_DESC InputElement_PosUV[2] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	D3D12_INPUT_LAYOUT_DESC VertexLayout_PosUV = { InputElement_PosUV, _countof( InputElement_PosUV ) };

// --------------------------------------------------------------------------------------------------------------------------------------------

	D3D12_INPUT_ELEMENT_DESC InputElement_Pos[1] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }};

	D3D12_INPUT_LAYOUT_DESC VertexLayout_Pos = { InputElement_Pos, _countof( InputElement_Pos ) };

// --------------------------------------------------------------------------------------------------------------------------------------------

	D3D12_INPUT_ELEMENT_DESC InputElement_LineColorThickness[3] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "THICKNESS", 0, DXGI_FORMAT_R32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	D3D12_INPUT_LAYOUT_DESC VertexLayout_LineColorThickness = { InputElement_LineColorThickness, _countof( InputElement_LineColorThickness ) };

// --------------------------------------------------------------------------------------------------------------------------------------------

	CommonResources::CommonResources( ID3D12GraphicsCommandList2* CommandList )
	{
		m_ScreenTriangle = new ScreenTriangle( CommandList );
		m_UniformQuad = new UniformQuad( CommandList );
		m_PointLightSphere = new PointLightSphere( CommandList );
		m_SpotLightCone = new SpotLightCone(CommandList);
		m_ResolveAlphaBlendedPSO = new ResolveAlphaBlendedPSO(CommandList);
		m_ResolveEditorSelectionPSO = new ResolveEditorSelectionPSO(CommandList);
		m_TonemapPSO = new TonemapPSO(CommandList);
		m_SpriteEditorPrimitivePSO = new SpriteEditorPrimitivePSO(CommandList);
		m_SpriteHitProxyPSO = new SpriteHitProxyPSO(CommandList);
		m_LightPassPSO = new LightPassPSO(CommandList);
		m_DebugLineThicknessPSO = new DebugLineThicknessPSO(CommandList);
		m_DebugLinePSO = new DebugLinePSO(CommandList);
		m_HZBPSO = new HZBPSO(CommandList);
	}

	CommonResources::~CommonResources()
	{
		delete m_ScreenTriangle;
		delete m_UniformQuad;
		delete m_PointLightSphere;
		delete m_SpotLightCone;
		delete m_ResolveAlphaBlendedPSO;
		delete m_ResolveEditorSelectionPSO;
		delete m_TonemapPSO;
		delete m_SpriteEditorPrimitivePSO;
		delete m_SpriteHitProxyPSO;
		delete m_LightPassPSO;
		delete m_DebugLineThicknessPSO;
		delete m_DebugLinePSO;
		delete m_HZBPSO;
	}

	void CommonResources::Init( ID3D12GraphicsCommandList2* CommandList )
	{
		if (!m_SingletonInstance)
		{
			m_SingletonInstance = new CommonResources( CommandList );
		}
	}

	void CommonResources::Shutdown()
	{
		if (m_SingletonInstance)
		{
			delete m_SingletonInstance;
			m_SingletonInstance = nullptr;
		}
	}

	float TriangleVertexData[] = 
	{
		1, -1, 0, 1, 1,
		-3, -1, 0, -1, 1,
		1, 3, 0, 1, -1
	};

	uint32 TriangleIndexData[] = { 0, 1, 2 };

	ScreenTriangle::ScreenTriangle( ID3D12GraphicsCommandList2* CommandList )
	{
		m_VertexBuffer = VertexBuffer::Create(CommandList, TriangleVertexData, 3, sizeof(TriangleVertexData) / 3, "ScreenTriangle");
		m_IndexBuffer = IndexBuffer::Create(CommandList, TriangleIndexData, 3, sizeof(TriangleIndexData), DXGI_FORMAT_R32_UINT, "ScreenTriangle");
	}

	ScreenTriangle::~ScreenTriangle()
	{
		if (m_VertexBuffer) { delete m_VertexBuffer; }
		if (m_IndexBuffer) { delete m_IndexBuffer; }
	}

	void ScreenTriangle::BindAndDraw( ID3D12GraphicsCommandList2* CommandList )
	{
		m_VertexBuffer->Bind(CommandList);
		m_IndexBuffer->Bind(CommandList);
		CommandList->DrawIndexedInstanced(m_IndexBuffer->m_IndexCount, 1, 0, 0, 0);
	}

// --------------------------------------------------------------------------------------

	float UniformQuadVertexData[] = 
	{
		0.5, -0.5, 0, 1, 1,
		-0.5, -0.5, 0, 0, 1,
		0.5, 0.5, 0, 1, 0,
		-0.5, 0.5, 0, 0, 0
	};

	uint32 UniformQuadIndexData[] = { 3, 1, 2, 2, 1, 0 };

	UniformQuad::UniformQuad( ID3D12GraphicsCommandList2* CommandList )
	{
		m_VertexBuffer = VertexBuffer::Create(CommandList, UniformQuadVertexData, 4, sizeof(UniformQuadVertexData) / 4, "UniformQuad");
		m_IndexBuffer = IndexBuffer::Create(CommandList, UniformQuadIndexData, 6, sizeof(UniformQuadIndexData), DXGI_FORMAT_R32_UINT, "UniformQuad");
	}

	UniformQuad::~UniformQuad()
	{
		if (m_VertexBuffer) { delete m_VertexBuffer; }
		if (m_IndexBuffer) { delete m_IndexBuffer; }
	}

	void UniformQuad::BindAndDraw( ID3D12GraphicsCommandList2* CommandList )
	{
		m_VertexBuffer->Bind(CommandList);
		m_IndexBuffer->Bind(CommandList);
		CommandList->DrawIndexedInstanced(m_IndexBuffer->m_IndexCount, 1, 0, 0, 0);
	}

// --------------------------------------------------------------------------------------

	PointLightSphere::PointLightSphere( ID3D12GraphicsCommandList2* CommandList )
	{
		par_shapes_mesh* SphereMesh = par_shapes_create_subdivided_sphere( 2 );

		const uint32 VertexCount = SphereMesh->npoints;
		const uint32 IndexCount = SphereMesh->ntriangles * 3;

		const uint32 IndexBufferSize = IndexCount * sizeof(PAR_SHAPES_T);

		m_VertexBuffer = VertexBuffer::Create(CommandList, SphereMesh->points, VertexCount, sizeof(float) * 3, "PointLightSphere");
		m_IndexBuffer = IndexBuffer::Create(CommandList, SphereMesh->triangles, IndexCount, IndexBufferSize, DXGI_FORMAT_R16_UINT, "PointLightSphere");

		par_shapes_free_mesh(SphereMesh);
	}

	PointLightSphere::~PointLightSphere()
	{
		if (m_VertexBuffer) { delete m_VertexBuffer; }
		if (m_IndexBuffer) { delete m_IndexBuffer; }
	}

	void PointLightSphere::BindAndDraw( ID3D12GraphicsCommandList2* CommandList )
	{
		m_VertexBuffer->Bind(CommandList);
		m_IndexBuffer->Bind(CommandList);
		CommandList->DrawIndexedInstanced(m_IndexBuffer->m_IndexCount, 1, 0, 0, 0);
	}

// --------------------------------------------------------------------------------------

	SpotLightCone::SpotLightCone( ID3D12GraphicsCommandList2* CommandList )
	{
		RenderGeometeryHelper::CreateSpotlightStencilGeometery(CommandList, m_VertexBuffer, m_IndexBuffer);
	}

	SpotLightCone::~SpotLightCone()
	{
		if (m_VertexBuffer) { delete m_VertexBuffer; }
		if (m_IndexBuffer) { delete m_IndexBuffer; }
	}

	void SpotLightCone::BindAndDraw( ID3D12GraphicsCommandList2* CommandList )
	{
		m_VertexBuffer->Bind(CommandList);
		m_IndexBuffer->Bind(CommandList);
		CommandList->DrawIndexedInstanced(m_IndexBuffer->m_IndexCount, 1, 0, 0, 0);
	}

// --------------------------------------------------------------------------------------

	ResolveAlphaBlendedPSO::ResolveAlphaBlendedPSO( ID3D12GraphicsCommandList2* CommandList )
	{
		m_PSO = nullptr;

		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		std::wstring ShaderPath = StringHelper::s2ws(Path::ConvertProjectPath( "\\Engine\\Content\\Shader\\ResolveAlphaBlended.hlsl" ));
		ID3DBlob* VertexShaderBlob;
		ID3DBlob* PixelShaderBlob;

		std::vector<const wchar_t*> Macros = {};
		CompileShader( ShaderPath, L"Main_VS", L"vs_6_6", Macros , &VertexShaderBlob);
		CompileShader( ShaderPath, L"Main_PS", L"ps_6_6", Macros , &PixelShaderBlob);

		CD3DX12_BLEND_DESC BlendDesc = {};
		BlendDesc.RenderTarget[0].BlendEnable = TRUE;
		BlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		BlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		BlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		BlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		BlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ZERO;
		BlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ONE;
		BlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

		D3D12_GRAPHICS_PIPELINE_STATE_DESC PipelineDesc = {};
		PipelineDesc.pRootSignature						= Renderer::Get()->m_BindlessRootSinature.Get();
		PipelineDesc.InputLayout						= VertexLayout_PosUV;
		PipelineDesc.PrimitiveTopologyType				= D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		PipelineDesc.RasterizerState					= CD3DX12_RASTERIZER_DESC( D3D12_DEFAULT );
		PipelineDesc.BlendState							= BlendDesc;
		PipelineDesc.DepthStencilState.DepthEnable		= FALSE;
		PipelineDesc.SampleMask							= UINT_MAX;
		PipelineDesc.VS									= CD3DX12_SHADER_BYTECODE(VertexShaderBlob);
		PipelineDesc.PS									= CD3DX12_SHADER_BYTECODE(PixelShaderBlob);
		PipelineDesc.DSVFormat							= DXGI_FORMAT_R32_FLOAT;
		PipelineDesc.NumRenderTargets					= 1;
		PipelineDesc.RTVFormats[0]						= DXGI_FORMAT_R8G8B8A8_UNORM;
		PipelineDesc.SampleDesc.Count					= 1;

		Device->CreateGraphicsPipelineState( &PipelineDesc, IID_PPV_ARGS( &m_PSO ) );

#if D3D12_Debug_INFO
		m_PSO->SetName(L"PSO_ResolveAlphaBlended");
#endif
	}

	ResolveAlphaBlendedPSO::~ResolveAlphaBlendedPSO()
	{
		m_PSO->Release();
	}

// --------------------------------------------------------------------------------------

	ResolveEditorSelectionPSO::ResolveEditorSelectionPSO( ID3D12GraphicsCommandList2* CommandList )
	{
		m_PSO = nullptr;

		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		std::wstring ShaderPath = StringHelper::s2ws( Path::ConvertProjectPath( "\\Engine\\Content\\Shader\\ResolveEditorSelection.hlsl" ) );
		ID3DBlob* VertexShaderBlob;
		ID3DBlob* PixelShaderBlob;

		std::vector<const wchar_t*> Macros = {};
		CompileShader( ShaderPath, L"Main_VS", L"vs_6_6", Macros, &VertexShaderBlob);
		CompileShader( ShaderPath, L"Main_PS", L"ps_6_6", Macros, &PixelShaderBlob);

		CD3DX12_BLEND_DESC BlendDesc = {};
		BlendDesc.RenderTarget[0].BlendEnable = TRUE;
		BlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		BlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		BlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		BlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		BlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ZERO;
		BlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ONE;
		BlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

		D3D12_GRAPHICS_PIPELINE_STATE_DESC PipelineDesc = {};
		PipelineDesc.pRootSignature						= Renderer::Get()->m_BindlessRootSinature.Get();
		PipelineDesc.InputLayout						= VertexLayout_PosUV;
		PipelineDesc.PrimitiveTopologyType				= D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		PipelineDesc.RasterizerState					= CD3DX12_RASTERIZER_DESC( D3D12_DEFAULT );
		PipelineDesc.BlendState							= BlendDesc;
		PipelineDesc.DepthStencilState.DepthEnable		= FALSE;
		PipelineDesc.SampleMask							= UINT_MAX;
		PipelineDesc.VS									= CD3DX12_SHADER_BYTECODE(VertexShaderBlob);
		PipelineDesc.PS									= CD3DX12_SHADER_BYTECODE(PixelShaderBlob);
		PipelineDesc.DSVFormat							= DXGI_FORMAT_R32_FLOAT;
		PipelineDesc.NumRenderTargets					= 1;
		PipelineDesc.RTVFormats[0]						= DXGI_FORMAT_R8G8B8A8_UNORM;
		PipelineDesc.SampleDesc.Count					= 1;

		Device->CreateGraphicsPipelineState( &PipelineDesc, IID_PPV_ARGS( &m_PSO ) );

#if D3D12_Debug_INFO
		m_PSO->SetName(L"PSO_ResolveEditorSelection");
#endif
	}

	ResolveEditorSelectionPSO::~ResolveEditorSelectionPSO()
	{
		m_PSO->Release();
	}

// --------------------------------------------------------------------------------------

	TonemapPSO::TonemapPSO( ID3D12GraphicsCommandList2* CommandList )
	{
		m_PSO = nullptr;

		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		std::wstring ShaderPath = StringHelper::s2ws( Path::ConvertProjectPath( "\\Engine\\Content\\Shader\\Tonemap.hlsl" ) );

		ID3DBlob* VertexShaderBlob;
		ID3DBlob* PixelShaderBlob;

		const std::vector<const wchar_t*> Macros = {};
		CompileShader( ShaderPath, L"Main_VS", L"vs_6_6", Macros, &VertexShaderBlob);
		CompileShader( ShaderPath, L"Main_PS", L"ps_6_6", Macros, &PixelShaderBlob);

		D3D12_GRAPHICS_PIPELINE_STATE_DESC PipelineDesc = {};
		PipelineDesc.pRootSignature						= Renderer::Get()->m_BindlessRootSinature.Get();
		PipelineDesc.InputLayout						= VertexLayout_PosUV;
		PipelineDesc.PrimitiveTopologyType				= D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		PipelineDesc.RasterizerState					= CD3DX12_RASTERIZER_DESC( D3D12_DEFAULT );
		PipelineDesc.BlendState							= CD3DX12_BLEND_DESC ( D3D12_DEFAULT );
		PipelineDesc.DepthStencilState.DepthEnable		= FALSE;
		PipelineDesc.SampleMask							= UINT_MAX;
		PipelineDesc.VS									= CD3DX12_SHADER_BYTECODE(VertexShaderBlob);
		PipelineDesc.PS									= CD3DX12_SHADER_BYTECODE(PixelShaderBlob);
		PipelineDesc.DSVFormat							= DXGI_FORMAT_R32_FLOAT;
		PipelineDesc.NumRenderTargets					= 1;
		PipelineDesc.RTVFormats[0]						= DISPLAY_OUTPUT_FORMAT;
		PipelineDesc.SampleDesc.Count					= 1;

		Device->CreateGraphicsPipelineState( &PipelineDesc, IID_PPV_ARGS( &m_PSO ) );

#if D3D12_Debug_INFO
		m_PSO->SetName(L"PSO_Tonemap");
#endif
	}

	TonemapPSO::~TonemapPSO()
	{
		m_PSO->Release();
	}

// --------------------------------------------------------------------------------------

	SpriteEditorPrimitivePSO::SpriteEditorPrimitivePSO( ID3D12GraphicsCommandList2* CommandList )
	{
		m_PSO = nullptr;

		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		ID3DBlob* VertexShaderBlob;
		ID3DBlob* PixelShaderBlob;

		std::vector<const wchar_t*> Macros = {};
		std::wstring ShaderPath = StringHelper::s2ws( Path::ConvertProjectPath( "\\Engine\\Content\\Shader\\Sprite.hlsl" ) );
		CompileShader( ShaderPath, L"Main_VS", L"vs_6_6", Macros, &VertexShaderBlob);
		CompileShader( ShaderPath, L"Main_PS", L"ps_6_6", Macros, &PixelShaderBlob);

		CD3DX12_BLEND_DESC BlendDesc = {};
		BlendDesc.RenderTarget[0].BlendEnable = TRUE;
		BlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		BlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		BlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		BlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		BlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ZERO;
		BlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ONE;
		BlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

		D3D12_GRAPHICS_PIPELINE_STATE_DESC PipelineDesc = {};
		PipelineDesc.pRootSignature						= Renderer::Get()->m_BindlessRootSinature.Get();
		PipelineDesc.InputLayout						= VertexLayout_PosUV;
		PipelineDesc.PrimitiveTopologyType				= D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		PipelineDesc.RasterizerState					= CD3DX12_RASTERIZER_DESC( D3D12_DEFAULT );
		//PipelineDesc.BlendState							= BlendDesc;
		PipelineDesc.BlendState							= CD3DX12_BLEND_DESC( D3D12_DEFAULT );
		PipelineDesc.DepthStencilState.DepthEnable		= TRUE;
		PipelineDesc.DepthStencilState.DepthWriteMask	= D3D12_DEPTH_WRITE_MASK_ALL;
		PipelineDesc.DepthStencilState.DepthFunc		= D3D12_COMPARISON_FUNC_GREATER;
		PipelineDesc.DepthStencilState.StencilEnable	= FALSE;
		PipelineDesc.SampleMask							= UINT_MAX;
		PipelineDesc.VS									= CD3DX12_SHADER_BYTECODE(VertexShaderBlob);
		PipelineDesc.PS									= CD3DX12_SHADER_BYTECODE(PixelShaderBlob);
		PipelineDesc.DSVFormat							= DEPTH_FORMAT;
		PipelineDesc.NumRenderTargets					= 1;
		PipelineDesc.RTVFormats[0]						= DXGI_FORMAT_R8G8B8A8_UNORM;
		PipelineDesc.SampleDesc.Count					= 1;

		Device->CreateGraphicsPipelineState( &PipelineDesc, IID_PPV_ARGS( &m_PSO ) );

#if D3D12_Debug_INFO
		m_PSO->SetName(L"PSO_SpriteEditorPrimitive");
#endif
	}

	SpriteEditorPrimitivePSO::~SpriteEditorPrimitivePSO()
	{
		m_PSO->Release();
	}

// --------------------------------------------------------------------------------------

	SpriteHitProxyPSO::SpriteHitProxyPSO( ID3D12GraphicsCommandList2* CommandList )
	{
		m_PSO = nullptr;

		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		std::wstring ShaderPath = StringHelper::s2ws( Path::ConvertProjectPath( "\\Engine\\Content\\Shader\\Sprite.hlsl" ) );

		ID3DBlob* VertexShaderBlob;
		ID3DBlob* PixelShaderBlob;

		std::vector<const wchar_t*> Macros = {L"HITPROXY_PASS=1"};

		CompileShader( ShaderPath, L"Main_VS", L"vs_6_6", Macros, &VertexShaderBlob);
		CompileShader( ShaderPath, L"Main_PS", L"ps_6_6", Macros, &PixelShaderBlob);

		CD3DX12_BLEND_DESC BlendDesc = {};
		BlendDesc.RenderTarget[0].BlendEnable = TRUE;
		BlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		BlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		BlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		BlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		BlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ZERO;
		BlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ONE;
		BlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

		D3D12_GRAPHICS_PIPELINE_STATE_DESC PipelineDesc = {};
		PipelineDesc.pRootSignature						= Renderer::Get()->m_BindlessRootSinature.Get();
		PipelineDesc.InputLayout						= VertexLayout_PosUV;
		PipelineDesc.PrimitiveTopologyType				= D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		PipelineDesc.RasterizerState					= CD3DX12_RASTERIZER_DESC( D3D12_DEFAULT );
		//PipelineDesc.BlendState							= BlendDesc;
		PipelineDesc.BlendState							= CD3DX12_BLEND_DESC( D3D12_DEFAULT );
		PipelineDesc.DepthStencilState.DepthEnable		= TRUE;
		PipelineDesc.DepthStencilState.DepthWriteMask	= D3D12_DEPTH_WRITE_MASK_ALL;
		PipelineDesc.DepthStencilState.DepthFunc		= D3D12_COMPARISON_FUNC_GREATER;
		PipelineDesc.DepthStencilState.StencilEnable	= FALSE;
		PipelineDesc.SampleMask							= UINT_MAX;
		PipelineDesc.VS									= CD3DX12_SHADER_BYTECODE(VertexShaderBlob);
		PipelineDesc.PS									= CD3DX12_SHADER_BYTECODE(PixelShaderBlob);
		PipelineDesc.DSVFormat							= DEPTH_FORMAT;
		PipelineDesc.NumRenderTargets					= 1;
		PipelineDesc.RTVFormats[0]						= GBUFFER_GUID_FORMAT;
		PipelineDesc.SampleDesc.Count					= 1;

		Device->CreateGraphicsPipelineState( &PipelineDesc, IID_PPV_ARGS( &m_PSO ) );

#if D3D12_Debug_INFO
		m_PSO->SetName(L"PSO_SpriteHitProxy");
#endif
	}

	SpriteHitProxyPSO::~SpriteHitProxyPSO()
	{
		m_PSO->Release();
	}

// --------------------------------------------------------------------------------------

	LightPassPSO::LightPassPSO( ID3D12GraphicsCommandList2* CommandList )
	{
		m_PSO = nullptr;

		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		std::wstring ShaderPath = StringHelper::s2ws( Path::ConvertProjectPath( "\\Engine\\Content\\Shader\\LightPassDeferred.hlsl" ) );

		ID3DBlob* VertexShaderBlob;
		ID3DBlob* PixelShaderBlob;

		std::wstring SideMacro = StringHelper::s2ws("SPOTLIGHT_STENCIL_SIDES=" + std::to_string(SPOTLIGHT_STENCIL_SIDES));
		std::wstring SliceMacro = StringHelper::s2ws("SPOTLIGHT_STENCIL_SLICES=" + std::to_string(SPOTLIGHT_STENCIL_SLICES));

		const std::vector<const wchar_t*> Macros = { SideMacro.c_str(), SliceMacro.c_str() };
		CompileShader( ShaderPath, L"Main_VS", L"vs_6_6", Macros, &VertexShaderBlob);
		CompileShader( ShaderPath, L"Main_PS", L"ps_6_6", Macros, &PixelShaderBlob);

		CD3DX12_BLEND_DESC BlendDesc = {};
		BlendDesc.RenderTarget[0].BlendEnable = TRUE;
		BlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		BlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
		BlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
		BlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		BlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
		BlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ONE;
		BlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

		CD3DX12_RASTERIZER_DESC RasterizerDesc(D3D12_DEFAULT);
		RasterizerDesc.CullMode = D3D12_CULL_MODE_FRONT;

		D3D12_GRAPHICS_PIPELINE_STATE_DESC PipelineDesc = {};
		PipelineDesc.pRootSignature						= Renderer::Get()->m_BindlessRootSinature.Get();
		PipelineDesc.InputLayout						= VertexLayout_Pos;
		PipelineDesc.PrimitiveTopologyType				= D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		PipelineDesc.RasterizerState					= RasterizerDesc;
		PipelineDesc.BlendState							= BlendDesc;
		PipelineDesc.DepthStencilState.DepthEnable		= FALSE;
		PipelineDesc.SampleMask							= UINT_MAX;
		PipelineDesc.VS									= CD3DX12_SHADER_BYTECODE(VertexShaderBlob);
		PipelineDesc.PS									= CD3DX12_SHADER_BYTECODE(PixelShaderBlob);
		PipelineDesc.DSVFormat							= DEPTH_FORMAT;
		PipelineDesc.NumRenderTargets					= 1;
		PipelineDesc.RTVFormats[0]						= GBUFFER_COLOR_DEFERRED_FORMAT;
		PipelineDesc.SampleDesc.Count					= 1;

		Device->CreateGraphicsPipelineState( &PipelineDesc, IID_PPV_ARGS( &m_PSO ) );

#if D3D12_Debug_INFO
		m_PSO->SetName(L"PSO_LightpassDeferred");
#endif
	}

	LightPassPSO::~LightPassPSO()
	{
		m_PSO->Release();
	}

// --------------------------------------------------------------------------------------

	DebugLineThicknessPSO::DebugLineThicknessPSO( ID3D12GraphicsCommandList2* CommandList )
	{
		m_PSO = nullptr;

		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		std::wstring ShaderPath = StringHelper::s2ws( Path::ConvertProjectPath( "\\Engine\\Content\\Shader\\DebugLineThicknessShader.hlsl" ) );

		ID3DBlob* VertexShaderBlob;
		ID3DBlob* PixelShaderBlob;
		ID3DBlob* GeometeryShaderBlob;

		const std::vector<const wchar_t*> Macros;
		CompileShader( ShaderPath, L"Main_VS", L"vs_6_6", Macros, &VertexShaderBlob);
		CompileShader( ShaderPath, L"Main_PS", L"ps_6_6", Macros, &PixelShaderBlob);
		CompileShader( ShaderPath, L"Main_GS", L"gs_6_6", Macros, &GeometeryShaderBlob);

		CD3DX12_RASTERIZER_DESC RasterizerDesc(D3D12_DEFAULT);
		RasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;

		D3D12_GRAPHICS_PIPELINE_STATE_DESC PipelineDesc = {};
		PipelineDesc.pRootSignature						= Renderer::Get()->m_BindlessRootSinature.Get();
		PipelineDesc.InputLayout						= VertexLayout_LineColorThickness;
		PipelineDesc.PrimitiveTopologyType				= D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
		PipelineDesc.RasterizerState					= RasterizerDesc;
		PipelineDesc.BlendState							= CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		PipelineDesc.DepthStencilState.DepthEnable		= TRUE;
		PipelineDesc.DepthStencilState.DepthWriteMask	= D3D12_DEPTH_WRITE_MASK_ALL;
		PipelineDesc.DepthStencilState.DepthFunc		= D3D12_COMPARISON_FUNC_GREATER;
		PipelineDesc.DepthStencilState.StencilEnable	= FALSE;
		PipelineDesc.SampleMask							= UINT_MAX;
		PipelineDesc.VS									= CD3DX12_SHADER_BYTECODE(VertexShaderBlob);
		PipelineDesc.PS									= CD3DX12_SHADER_BYTECODE(PixelShaderBlob);
		PipelineDesc.GS									= CD3DX12_SHADER_BYTECODE(GeometeryShaderBlob);
		PipelineDesc.DSVFormat							= DEPTH_FORMAT;
		PipelineDesc.NumRenderTargets					= 1;
		PipelineDesc.RTVFormats[0]						= GBUFFER_BASE_COLOR_FORMAT;
		PipelineDesc.SampleDesc.Count					= 1;

		Device->CreateGraphicsPipelineState( &PipelineDesc, IID_PPV_ARGS( &m_PSO ) );

#if D3D12_Debug_INFO
		m_PSO->SetName(L"PSO_DebugLineThickness");
#endif
	}

	DebugLineThicknessPSO::~DebugLineThicknessPSO()
	{
		m_PSO->Release();
	}

// --------------------------------------------------------------------------------------

	DebugLinePSO::DebugLinePSO( ID3D12GraphicsCommandList2* CommandList )
	{
		m_PSO = nullptr;

		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		std::wstring ShaderPath = StringHelper::s2ws( Path::ConvertProjectPath( "\\Engine\\Content\\Shader\\DebugLineShader.hlsl" ) );

		ID3DBlob* VertexShaderBlob;
		ID3DBlob* PixelShaderBlob;

		const std::vector<const wchar_t*> Macros;
		CompileShader( ShaderPath, L"Main_VS", L"vs_6_6", Macros, &VertexShaderBlob);
		CompileShader( ShaderPath, L"Main_PS", L"ps_6_6", Macros, &PixelShaderBlob);

		CD3DX12_RASTERIZER_DESC RasterizerDesc(D3D12_DEFAULT);
		RasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;

		D3D12_GRAPHICS_PIPELINE_STATE_DESC PipelineDesc = {};
		PipelineDesc.pRootSignature						= Renderer::Get()->m_BindlessRootSinature.Get();
		PipelineDesc.InputLayout						= VertexLayout_LineColorThickness;
		PipelineDesc.PrimitiveTopologyType				= D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
		PipelineDesc.RasterizerState					= RasterizerDesc;
		PipelineDesc.BlendState							= CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		PipelineDesc.DepthStencilState.DepthEnable		= TRUE;
		PipelineDesc.DepthStencilState.DepthWriteMask	= D3D12_DEPTH_WRITE_MASK_ALL;
		PipelineDesc.DepthStencilState.DepthFunc		= D3D12_COMPARISON_FUNC_GREATER;
		PipelineDesc.DepthStencilState.StencilEnable	= FALSE;
		PipelineDesc.SampleMask							= UINT_MAX;
		PipelineDesc.VS									= CD3DX12_SHADER_BYTECODE(VertexShaderBlob);
		PipelineDesc.PS									= CD3DX12_SHADER_BYTECODE(PixelShaderBlob);
		PipelineDesc.DSVFormat							= DEPTH_FORMAT;
		PipelineDesc.NumRenderTargets					= 1;
		PipelineDesc.RTVFormats[0]						= GBUFFER_BASE_COLOR_FORMAT;
		PipelineDesc.SampleDesc.Count					= 1;

		Device->CreateGraphicsPipelineState( &PipelineDesc, IID_PPV_ARGS( &m_PSO ) );

#if D3D12_Debug_INFO
		m_PSO->SetName(L"PSO_DebugLine");
#endif
	}

	DebugLinePSO::~DebugLinePSO()
	{
		m_PSO->Release();
	}

// --------------------------------------------------------------------------------------

	HZBPSO::HZBPSO( ID3D12GraphicsCommandList2* CommandList )
	{
		m_PSO = nullptr;

		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		std::wstring ShaderPath = StringHelper::s2ws( Path::ConvertProjectPath( "\\Engine\\Content\\Shader\\HZB.hlsl" ) );
		ID3DBlob* ComputeShaderBlob;
		const std::vector<const wchar_t*> Macros;
		CompileShader( ShaderPath, L"Main_CS", L"cs_6_6", Macros, &ComputeShaderBlob);

		D3D12_COMPUTE_PIPELINE_STATE_DESC PipelineDesc = {};
		PipelineDesc.pRootSignature						= Renderer::Get()->m_BindlessRootSinature.Get();
		PipelineDesc.CS									= CD3DX12_SHADER_BYTECODE(ComputeShaderBlob);

		Device->CreateComputePipelineState( &PipelineDesc, IID_PPV_ARGS( &m_PSO ) );

#if D3D12_Debug_INFO
		m_PSO->SetName(L"PSO_DebugLine");
#endif
	}

	HZBPSO::~HZBPSO()
	{
		m_PSO->Release();
	}

// --------------------------------------------------------------------------------------

	void CompileShaderString( const std::wstring& ShaderPath, const char* EntryPoint, const char* Profile, ID3DBlob*& ShaderBlob, const D3D_SHADER_MACRO* Macros)
	{
		UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
		flags |= D3DCOMPILE_DEBUG;
#endif

		ID3DBlob* errorBlob = nullptr;
		HRESULT hr = D3DCompileFromFile( ShaderPath.c_str(), Macros, D3D_COMPILE_STANDARD_FILE_INCLUDE, 
			EntryPoint, Profile, flags, 0, &ShaderBlob, &errorBlob );

		if ( FAILED(hr) )
		{
			if ( errorBlob )
			{
				LOG(LogCommonResources, Error, "Shader compile failed. %s", (char*)errorBlob->GetBufferPointer());
				errorBlob->Release();
			}

			if ( ShaderBlob )
				ShaderBlob->Release();
		}
	}

	bool CompileShader( const std::wstring& ShaderPath, const wchar_t* EntryPoint, const wchar_t* Profile, const std::vector<const wchar_t*>& Macros, ID3DBlob** ByteBlob )
	{
		Microsoft::WRL::ComPtr<IDxcUtils> pUtils;
		Microsoft::WRL::ComPtr<IDxcCompiler3> pCompiler;

		DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(pUtils.GetAddressOf()));
		DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(pCompiler.GetAddressOf()));

		Microsoft::WRL::ComPtr<IDxcIncludeHandler> pIncludeHandler;
		pUtils->CreateDefaultIncludeHandler(pIncludeHandler.GetAddressOf());

		std::vector<LPCWSTR> Args;
		Args.push_back(L"Name");

		Args.push_back(L"-E");
		Args.push_back(EntryPoint);

		Args.push_back(L"-T");
		Args.push_back(Profile);

		Args.push_back(L"-Zs");

		for (auto& Mac : Macros)
		{
			Args.push_back(L"-D");
			Args.push_back(Mac);
		}

		//Args.push_back(L"-Fo");
		//Args.push_back( L"myshader.bin");
		//
		//Args.push_back(L"-Fd");
		//Args.push_back(L"myshader.pdb");

		Args.push_back(L"-Qstrip_reflect");

		Microsoft::WRL::ComPtr<IDxcBlobEncoding> pSource = nullptr;
		pUtils->LoadFile(ShaderPath.c_str(), nullptr, &pSource);
		DxcBuffer Source;
		Source.Ptr = pSource->GetBufferPointer();
		Source.Size = pSource->GetBufferSize();
		Source.Encoding = DXC_CP_ACP;

		Microsoft::WRL::ComPtr<IDxcResult> pResults;
		pCompiler->Compile(
			&Source,
			Args.data(),
			Args.size(),
			pIncludeHandler.Get(),
			IID_PPV_ARGS(pResults.GetAddressOf())
		);

		Microsoft::WRL::ComPtr<IDxcBlobUtf8> pErrors = nullptr;
		pResults->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&pErrors), nullptr);
		if (pErrors != nullptr && pErrors->GetStringLength() != 0)
		{
			LOG(LogCommonResources, Warning, "\"%ws\" complation log:\n%s", ShaderPath.c_str(), pErrors->GetStringPointer());
		}

		HRESULT hrStatus;
		pResults->GetStatus(&hrStatus);
		if (FAILED(hrStatus))
		{
			LOG(LogCommonResources, Error, "shader complation Failed.");
			return false;
		}

		Microsoft::WRL::ComPtr<IDxcBlob> pShader = nullptr;
		Microsoft::WRL::ComPtr<IDxcBlobUtf16> pShaderName = nullptr;
		pResults->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&pShader), &pShaderName);

		D3DCreateBlob(pShader->GetBufferSize(), ByteBlob);
		memcpy((*ByteBlob)->GetBufferPointer(), pShader->GetBufferPointer(), pShader->GetBufferSize());

		return true;
	}


 }  // namespace Drn