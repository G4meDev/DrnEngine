#include "DrnPCH.h"
#include "PipelineStateObject.h"

namespace Drn
{
	PipelineStateObject::PipelineStateObject()
		: SimpleRenderResource()
		, m_PipelineState(nullptr)
	{}

	PipelineStateObject::~PipelineStateObject()
	{}


	PipelineStateObject* PipelineStateObject::CreateMainPassPSO(D3D12_CULL_MODE CullMode, EInputLayoutType InputLayoutType,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE PrimitiveType, const ShaderBlob& Shaders)
	{
		PipelineStateObject* Result = new PipelineStateObject();

		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		D3D12_RASTERIZER_DESC RasterizerDesc = CD3DX12_RASTERIZER_DESC( D3D12_DEFAULT );
		RasterizerDesc.CullMode = CullMode;

		D3D12_GRAPHICS_PIPELINE_STATE_DESC PipelineDesc = {};
		PipelineDesc.pRootSignature						= Renderer::Get()->m_BindlessRootSinature.Get();
		PipelineDesc.InputLayout						= InputLayout::GetLayoutDescriptionForType(InputLayoutType);
		PipelineDesc.PrimitiveTopologyType				= PrimitiveType;
		PipelineDesc.RasterizerState					= RasterizerDesc;
		PipelineDesc.BlendState							= CD3DX12_BLEND_DESC( D3D12_DEFAULT );
		PipelineDesc.DepthStencilState.DepthEnable		= TRUE;
		PipelineDesc.DepthStencilState.DepthWriteMask	= D3D12_DEPTH_WRITE_MASK_ZERO;
		PipelineDesc.DepthStencilState.DepthFunc		= D3D12_COMPARISON_FUNC_GREATER_EQUAL;
		PipelineDesc.DepthStencilState.StencilEnable	= FALSE;
		PipelineDesc.SampleMask							= UINT_MAX;
		if (Shaders.m_VS) PipelineDesc.VS				= CD3DX12_SHADER_BYTECODE(Shaders.m_VS);
		if (Shaders.m_PS) PipelineDesc.PS				= CD3DX12_SHADER_BYTECODE(Shaders.m_PS);
		if (Shaders.m_GS) PipelineDesc.GS				= CD3DX12_SHADER_BYTECODE(Shaders.m_GS);
		if (Shaders.m_HS) PipelineDesc.HS				= CD3DX12_SHADER_BYTECODE(Shaders.m_HS);
		if (Shaders.m_DS) PipelineDesc.DS				= CD3DX12_SHADER_BYTECODE(Shaders.m_DS);
		PipelineDesc.DSVFormat							= DEPTH_FORMAT;
		PipelineDesc.NumRenderTargets					= 6;
		PipelineDesc.RTVFormats[0]						= GBUFFER_COLOR_DEFERRED_FORMAT;
		PipelineDesc.RTVFormats[1]						= GBUFFER_BASE_COLOR_FORMAT;
		PipelineDesc.RTVFormats[2]						= GBUFFER_WORLD_NORMAL_FORMAT;
		PipelineDesc.RTVFormats[3]						= GBUFFER_MASKS_FORMAT;
		PipelineDesc.RTVFormats[4]						= GBUFFER_MASKS_FORMAT;
		PipelineDesc.RTVFormats[5]						= GBUFFER_VELOCITY_FORMAT;
		PipelineDesc.SampleDesc.Count					= 1;

		Device->CreateGraphicsPipelineState( &PipelineDesc, IID_PPV_ARGS( Result->m_PipelineState.GetAddressOf() ) );
		return Result;
	}

	PipelineStateObject* PipelineStateObject::CreatePrePassPSO(D3D12_CULL_MODE CullMode, EInputLayoutType InputLayoutType, const ShaderBlob& Shaders)
	{
		PipelineStateObject* Result = new PipelineStateObject();

		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		D3D12_RASTERIZER_DESC RasterizerDesc = CD3DX12_RASTERIZER_DESC( D3D12_DEFAULT );
		RasterizerDesc.CullMode = CullMode;

		D3D12_GRAPHICS_PIPELINE_STATE_DESC PipelineDesc = {};
		PipelineDesc.pRootSignature						= Renderer::Get()->m_BindlessRootSinature.Get();
		PipelineDesc.InputLayout						= InputLayout::GetLayoutDescriptionForType(InputLayoutType);
		PipelineDesc.PrimitiveTopologyType				= D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		PipelineDesc.RasterizerState					= RasterizerDesc;
		PipelineDesc.BlendState							= CD3DX12_BLEND_DESC( D3D12_DEFAULT );
		PipelineDesc.DepthStencilState.DepthEnable		= TRUE;
		PipelineDesc.DepthStencilState.DepthWriteMask	= D3D12_DEPTH_WRITE_MASK_ALL;
		PipelineDesc.DepthStencilState.DepthFunc		= D3D12_COMPARISON_FUNC_GREATER_EQUAL;
		PipelineDesc.DepthStencilState.StencilEnable	= FALSE;
		PipelineDesc.SampleMask							= UINT_MAX;
		if (Shaders.m_VS) PipelineDesc.VS				= CD3DX12_SHADER_BYTECODE(Shaders.m_VS);
		if (Shaders.m_PS) PipelineDesc.PS				= CD3DX12_SHADER_BYTECODE(Shaders.m_PS);
		if (Shaders.m_GS) PipelineDesc.GS				= CD3DX12_SHADER_BYTECODE(Shaders.m_GS);
		if (Shaders.m_HS) PipelineDesc.HS				= CD3DX12_SHADER_BYTECODE(Shaders.m_HS);
		if (Shaders.m_DS) PipelineDesc.DS				= CD3DX12_SHADER_BYTECODE(Shaders.m_DS);
		PipelineDesc.DSVFormat							= DEPTH_FORMAT;
		PipelineDesc.NumRenderTargets					= 0;
		PipelineDesc.SampleDesc.Count					= 1;

		Device->CreateGraphicsPipelineState( &PipelineDesc, IID_PPV_ARGS( Result->m_PipelineState.GetAddressOf() ) );
		return Result;
	}

	PipelineStateObject* PipelineStateObject::CreateDecalPassPSO(const ShaderBlob& Shaders)
	{
		PipelineStateObject* Result = new PipelineStateObject();

		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		D3D12_RASTERIZER_DESC RasterizerDesc = CD3DX12_RASTERIZER_DESC( D3D12_DEFAULT );
		RasterizerDesc.CullMode = D3D12_CULL_MODE_FRONT;

		CD3DX12_BLEND_DESC BlendDesc = {};
		BlendDesc.IndependentBlendEnable = TRUE;

		BlendDesc.RenderTarget[0].BlendEnable = TRUE;
		BlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		BlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		BlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		BlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		BlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ZERO;
		BlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
		BlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

		BlendDesc.RenderTarget[1].BlendEnable = TRUE;
		BlendDesc.RenderTarget[1].BlendOp = D3D12_BLEND_OP_ADD;
		BlendDesc.RenderTarget[1].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		BlendDesc.RenderTarget[1].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		BlendDesc.RenderTarget[1].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		BlendDesc.RenderTarget[1].SrcBlendAlpha = D3D12_BLEND_ZERO;
		BlendDesc.RenderTarget[1].DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
		BlendDesc.RenderTarget[1].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

		BlendDesc.RenderTarget[2].BlendEnable = TRUE;
		BlendDesc.RenderTarget[2].BlendOp = D3D12_BLEND_OP_ADD;
		BlendDesc.RenderTarget[2].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		BlendDesc.RenderTarget[2].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		BlendDesc.RenderTarget[2].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		BlendDesc.RenderTarget[2].SrcBlendAlpha = D3D12_BLEND_ZERO;
		BlendDesc.RenderTarget[2].DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
		BlendDesc.RenderTarget[2].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

		D3D12_GRAPHICS_PIPELINE_STATE_DESC PipelineDesc = {};
		PipelineDesc.pRootSignature						= Renderer::Get()->m_BindlessRootSinature.Get();
		PipelineDesc.InputLayout						= InputLayout::GetLayoutDescriptionForType(EInputLayoutType::Position);
		PipelineDesc.PrimitiveTopologyType				= D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		PipelineDesc.RasterizerState					= RasterizerDesc;
		PipelineDesc.BlendState							= BlendDesc;
		PipelineDesc.DepthStencilState.DepthEnable		= FALSE;
		PipelineDesc.DepthStencilState.DepthWriteMask	= D3D12_DEPTH_WRITE_MASK_ZERO;
		PipelineDesc.DepthStencilState.DepthFunc		= D3D12_COMPARISON_FUNC_ALWAYS;
		PipelineDesc.DepthStencilState.StencilEnable	= FALSE;
		PipelineDesc.SampleMask							= UINT_MAX;
		if (Shaders.m_VS) PipelineDesc.VS				= CD3DX12_SHADER_BYTECODE(Shaders.m_VS);
		if (Shaders.m_PS) PipelineDesc.PS				= CD3DX12_SHADER_BYTECODE(Shaders.m_PS);
		PipelineDesc.NumRenderTargets					= 3;
		PipelineDesc.RTVFormats[0]						= DECAL_BASE_COLOR_FORMAT;
		PipelineDesc.RTVFormats[1]						= DECAL_NORMAL_FORMAT;
		PipelineDesc.RTVFormats[2]						= DECAL_MASKS_FORMAT;
		PipelineDesc.SampleDesc.Count					= 1;

		Device->CreateGraphicsPipelineState( &PipelineDesc, IID_PPV_ARGS( Result->m_PipelineState.GetAddressOf() ) );
		return Result;
	}

	PipelineStateObject* PipelineStateObject::CreateMeshDecalPassPSO(D3D12_CULL_MODE CullMode, const ShaderBlob& Shaders)
	{
		PipelineStateObject* Result = new PipelineStateObject();

		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		D3D12_RASTERIZER_DESC RasterizerDesc = CD3DX12_RASTERIZER_DESC( D3D12_DEFAULT );
		RasterizerDesc.CullMode = CullMode;

		CD3DX12_BLEND_DESC BlendDesc = {};
		BlendDesc.IndependentBlendEnable = TRUE;

		BlendDesc.RenderTarget[0].BlendEnable = TRUE;
		BlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		BlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		BlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		BlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		BlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ZERO;
		BlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
		BlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

		BlendDesc.RenderTarget[1].BlendEnable = TRUE;
		BlendDesc.RenderTarget[1].BlendOp = D3D12_BLEND_OP_ADD;
		BlendDesc.RenderTarget[1].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		BlendDesc.RenderTarget[1].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		BlendDesc.RenderTarget[1].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		BlendDesc.RenderTarget[1].SrcBlendAlpha = D3D12_BLEND_ZERO;
		BlendDesc.RenderTarget[1].DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
		BlendDesc.RenderTarget[1].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

		BlendDesc.RenderTarget[2].BlendEnable = TRUE;
		BlendDesc.RenderTarget[2].BlendOp = D3D12_BLEND_OP_ADD;
		BlendDesc.RenderTarget[2].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		BlendDesc.RenderTarget[2].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		BlendDesc.RenderTarget[2].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		BlendDesc.RenderTarget[2].SrcBlendAlpha = D3D12_BLEND_ZERO;
		BlendDesc.RenderTarget[2].DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
		BlendDesc.RenderTarget[2].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

		D3D12_GRAPHICS_PIPELINE_STATE_DESC PipelineDesc = {};
		PipelineDesc.pRootSignature						= Renderer::Get()->m_BindlessRootSinature.Get();
		PipelineDesc.InputLayout						= InputLayout::GetLayoutDescriptionForType(EInputLayoutType::StandardMesh);
		PipelineDesc.PrimitiveTopologyType				= D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		PipelineDesc.RasterizerState					= RasterizerDesc;
		PipelineDesc.BlendState							= BlendDesc;
		PipelineDesc.DepthStencilState.DepthEnable		= TRUE;
		PipelineDesc.DepthStencilState.DepthWriteMask	= D3D12_DEPTH_WRITE_MASK_ZERO;
		PipelineDesc.DepthStencilState.DepthFunc		= D3D12_COMPARISON_FUNC_GREATER_EQUAL;
		PipelineDesc.DepthStencilState.StencilEnable	= FALSE;
		PipelineDesc.SampleMask							= UINT_MAX;
		if (Shaders.m_VS) PipelineDesc.VS				= CD3DX12_SHADER_BYTECODE(Shaders.m_VS);
		if (Shaders.m_PS) PipelineDesc.PS				= CD3DX12_SHADER_BYTECODE(Shaders.m_PS);
		PipelineDesc.NumRenderTargets					= 3;
		PipelineDesc.DSVFormat							= DEPTH_FORMAT;
		PipelineDesc.RTVFormats[0]						= DECAL_BASE_COLOR_FORMAT;
		PipelineDesc.RTVFormats[1]						= DECAL_NORMAL_FORMAT;
		PipelineDesc.RTVFormats[2]						= DECAL_MASKS_FORMAT;
		PipelineDesc.SampleDesc.Count					= 1;

		Device->CreateGraphicsPipelineState( &PipelineDesc, IID_PPV_ARGS( Result->m_PipelineState.GetAddressOf() ) );
		return Result;
	}


	PipelineStateObject* PipelineStateObject::CreatePointLightShadowDepthPassPSO(
		D3D12_CULL_MODE CullMode, EInputLayoutType InputLayoutType,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE PrimitiveType, const ShaderBlob& Shaders )
	{
		PipelineStateObject* Result = new PipelineStateObject();

		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		D3D12_RASTERIZER_DESC RasterizerDesc = CD3DX12_RASTERIZER_DESC( D3D12_DEFAULT );
		RasterizerDesc.CullMode = CullMode;

		D3D12_GRAPHICS_PIPELINE_STATE_DESC PipelineDesc = {};
		PipelineDesc.pRootSignature						= Renderer::Get()->m_BindlessRootSinature.Get();
		PipelineDesc.InputLayout						= InputLayout::GetLayoutDescriptionForType(InputLayoutType);
		PipelineDesc.PrimitiveTopologyType				= PrimitiveType;
		PipelineDesc.RasterizerState					= RasterizerDesc;
		PipelineDesc.BlendState							= CD3DX12_BLEND_DESC( D3D12_DEFAULT );
		PipelineDesc.DepthStencilState.DepthEnable		= TRUE;
		PipelineDesc.DepthStencilState.DepthWriteMask	= D3D12_DEPTH_WRITE_MASK_ALL;
		PipelineDesc.DepthStencilState.DepthFunc		= D3D12_COMPARISON_FUNC_LESS_EQUAL;
		PipelineDesc.SampleMask							= UINT_MAX;
		if (Shaders.m_VS) PipelineDesc.VS				= CD3DX12_SHADER_BYTECODE(Shaders.m_VS);
		if (Shaders.m_PS) PipelineDesc.PS				= CD3DX12_SHADER_BYTECODE(Shaders.m_PS);
		if (Shaders.m_GS) PipelineDesc.GS				= CD3DX12_SHADER_BYTECODE(Shaders.m_GS);
		if (Shaders.m_HS) PipelineDesc.HS				= CD3DX12_SHADER_BYTECODE(Shaders.m_HS);
		if (Shaders.m_DS) PipelineDesc.DS				= CD3DX12_SHADER_BYTECODE(Shaders.m_DS);
		PipelineDesc.DSVFormat							= DXGI_FORMAT_D16_UNORM;
		PipelineDesc.NumRenderTargets					= 0;
		PipelineDesc.SampleDesc.Count					= 1;

		Device->CreateGraphicsPipelineState( &PipelineDesc, IID_PPV_ARGS( Result->m_PipelineState.GetAddressOf() ) );
		return Result;
	}

	PipelineStateObject* PipelineStateObject::CreateSpotLightShadowDepthPassPSO(
		D3D12_CULL_MODE CullMode, EInputLayoutType InputLayoutType,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE PrimitiveType, const ShaderBlob& Shaders )
	{
		PipelineStateObject* Result = new PipelineStateObject();

		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		D3D12_RASTERIZER_DESC RasterizerDesc = CD3DX12_RASTERIZER_DESC( D3D12_DEFAULT );
		RasterizerDesc.CullMode = CullMode;

		D3D12_GRAPHICS_PIPELINE_STATE_DESC PipelineDesc = {};
		PipelineDesc.pRootSignature						= Renderer::Get()->m_BindlessRootSinature.Get();
		PipelineDesc.InputLayout						= InputLayout::GetLayoutDescriptionForType(InputLayoutType);
		PipelineDesc.PrimitiveTopologyType				= PrimitiveType;
		PipelineDesc.RasterizerState					= RasterizerDesc;
		PipelineDesc.BlendState							= CD3DX12_BLEND_DESC( D3D12_DEFAULT );
		PipelineDesc.DepthStencilState.DepthEnable		= TRUE;
		PipelineDesc.DepthStencilState.DepthWriteMask	= D3D12_DEPTH_WRITE_MASK_ALL;
		PipelineDesc.DepthStencilState.DepthFunc		= D3D12_COMPARISON_FUNC_LESS_EQUAL;
		PipelineDesc.SampleMask							= UINT_MAX;
		if (Shaders.m_VS) PipelineDesc.VS				= CD3DX12_SHADER_BYTECODE(Shaders.m_VS);
		if (Shaders.m_PS) PipelineDesc.PS				= CD3DX12_SHADER_BYTECODE(Shaders.m_PS);
		if (Shaders.m_GS) PipelineDesc.GS				= CD3DX12_SHADER_BYTECODE(Shaders.m_GS);
		if (Shaders.m_HS) PipelineDesc.HS				= CD3DX12_SHADER_BYTECODE(Shaders.m_HS);
		if (Shaders.m_DS) PipelineDesc.DS				= CD3DX12_SHADER_BYTECODE(Shaders.m_DS);
		PipelineDesc.DSVFormat							= DXGI_FORMAT_D16_UNORM;
		PipelineDesc.NumRenderTargets					= 0;
		PipelineDesc.SampleDesc.Count					= 1;

		Device->CreateGraphicsPipelineState( &PipelineDesc, IID_PPV_ARGS( Result->m_PipelineState.GetAddressOf() ) );
		return Result;
	}

	PipelineStateObject* PipelineStateObject::CreateSelectionPassPSO(
		D3D12_CULL_MODE CullMode, EInputLayoutType InputLayoutType,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE PrimitiveType, const ShaderBlob& Shaders)
	{
		PipelineStateObject* Result = new PipelineStateObject();

		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		D3D12_RASTERIZER_DESC RasterizerDesc = CD3DX12_RASTERIZER_DESC( D3D12_DEFAULT );
		RasterizerDesc.CullMode = CullMode;

		D3D12_DEPTH_STENCILOP_DESC StencilDesc = {};
		StencilDesc.StencilDepthFailOp = D3D12_STENCIL_OP_REPLACE;
		StencilDesc.StencilFailOp = D3D12_STENCIL_OP_REPLACE;
		StencilDesc.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
		StencilDesc.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

		D3D12_GRAPHICS_PIPELINE_STATE_DESC PipelineDesc = {};
		PipelineDesc.pRootSignature						= Renderer::Get()->m_BindlessRootSinature.Get();
		PipelineDesc.InputLayout						= InputLayout::GetLayoutDescriptionForType(InputLayoutType);
		PipelineDesc.PrimitiveTopologyType				= PrimitiveType;
		PipelineDesc.RasterizerState					= RasterizerDesc;
		PipelineDesc.BlendState							= CD3DX12_BLEND_DESC( D3D12_DEFAULT );
		PipelineDesc.DepthStencilState.DepthEnable		= TRUE;
		PipelineDesc.DepthStencilState.DepthWriteMask	= D3D12_DEPTH_WRITE_MASK_ALL;
		PipelineDesc.DepthStencilState.DepthFunc		= D3D12_COMPARISON_FUNC_GREATER;
		PipelineDesc.DepthStencilState.StencilEnable	= TRUE;
		PipelineDesc.DepthStencilState.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
		PipelineDesc.DepthStencilState.StencilReadMask	= D3D12_DEFAULT_STENCIL_READ_MASK;
		PipelineDesc.DepthStencilState.FrontFace		= StencilDesc;
		PipelineDesc.DepthStencilState.BackFace			= StencilDesc;
		PipelineDesc.SampleMask							= UINT_MAX;
		if (Shaders.m_VS) PipelineDesc.VS				= CD3DX12_SHADER_BYTECODE(Shaders.m_VS);
		if (Shaders.m_GS) PipelineDesc.GS				= CD3DX12_SHADER_BYTECODE(Shaders.m_GS);
		if (Shaders.m_HS) PipelineDesc.HS				= CD3DX12_SHADER_BYTECODE(Shaders.m_HS);
		if (Shaders.m_DS) PipelineDesc.DS				= CD3DX12_SHADER_BYTECODE(Shaders.m_DS);
		PipelineDesc.DSVFormat							= DEPTH_STENCIL_FORMAT;
		PipelineDesc.NumRenderTargets					= 0;
		PipelineDesc.SampleDesc.Count					= 1;

		Device->CreateGraphicsPipelineState( &PipelineDesc, IID_PPV_ARGS( Result->m_PipelineState.GetAddressOf() ) );
		return Result;
	}


	PipelineStateObject* PipelineStateObject::CreateHitProxyPassPSO(D3D12_CULL_MODE CullMode,
		EInputLayoutType InputLayoutType, D3D12_PRIMITIVE_TOPOLOGY_TYPE PrimitiveType, const ShaderBlob& Shaders )
	{
		PipelineStateObject* Result = new PipelineStateObject();

		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		DXGI_FORMAT backBufferFormat  = GBUFFER_GUID_FORMAT;
		DXGI_FORMAT depthBufferFormat = DXGI_FORMAT_D32_FLOAT;

		D3D12_RASTERIZER_DESC RasterizerDesc = CD3DX12_RASTERIZER_DESC( D3D12_DEFAULT );
		RasterizerDesc.CullMode = CullMode;

		D3D12_GRAPHICS_PIPELINE_STATE_DESC PipelineDesc = {};
		PipelineDesc.pRootSignature						= Renderer::Get()->m_BindlessRootSinature.Get();
		PipelineDesc.InputLayout						= InputLayout::GetLayoutDescriptionForType(InputLayoutType);
		PipelineDesc.PrimitiveTopologyType				= PrimitiveType;
		PipelineDesc.RasterizerState					= RasterizerDesc;
		PipelineDesc.BlendState							= CD3DX12_BLEND_DESC( D3D12_DEFAULT );
		PipelineDesc.DepthStencilState.DepthEnable		= TRUE;
		PipelineDesc.DepthStencilState.DepthWriteMask	= D3D12_DEPTH_WRITE_MASK_ALL;
		PipelineDesc.DepthStencilState.DepthFunc		= D3D12_COMPARISON_FUNC_GREATER;
		PipelineDesc.DepthStencilState.StencilEnable	= FALSE;
		PipelineDesc.SampleMask							= UINT_MAX;
		if (Shaders.m_VS) PipelineDesc.VS				= CD3DX12_SHADER_BYTECODE(Shaders.m_VS);
		if (Shaders.m_PS) PipelineDesc.PS				= CD3DX12_SHADER_BYTECODE(Shaders.m_PS);
		if (Shaders.m_GS) PipelineDesc.GS				= CD3DX12_SHADER_BYTECODE(Shaders.m_GS);
		if (Shaders.m_HS) PipelineDesc.HS				= CD3DX12_SHADER_BYTECODE(Shaders.m_HS);
		if (Shaders.m_DS) PipelineDesc.DS				= CD3DX12_SHADER_BYTECODE(Shaders.m_DS);
		PipelineDesc.DSVFormat							= depthBufferFormat;
		PipelineDesc.NumRenderTargets					= 1;
		PipelineDesc.RTVFormats[0]						= GBUFFER_GUID_FORMAT;
		PipelineDesc.SampleDesc.Count					= 1;

		Device->CreateGraphicsPipelineState( &PipelineDesc, IID_PPV_ARGS( Result->m_PipelineState.GetAddressOf() ) );
		return Result;
	}

	PipelineStateObject* PipelineStateObject::CreateEditorPrimitivePassPSO(D3D12_CULL_MODE CullMode,
		EInputLayoutType InputLayoutType, D3D12_PRIMITIVE_TOPOLOGY_TYPE PrimitiveType, const ShaderBlob& Shaders )
	{
		PipelineStateObject* Result = new PipelineStateObject();

		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		D3D12_RASTERIZER_DESC RasterizerDesc = CD3DX12_RASTERIZER_DESC( D3D12_DEFAULT );
		RasterizerDesc.CullMode = CullMode;

		D3D12_GRAPHICS_PIPELINE_STATE_DESC PipelineDesc = {};
		PipelineDesc.pRootSignature						= Renderer::Get()->m_BindlessRootSinature.Get();
		PipelineDesc.InputLayout						= InputLayout::GetLayoutDescriptionForType(InputLayoutType);
		PipelineDesc.PrimitiveTopologyType				= PrimitiveType;
		PipelineDesc.RasterizerState					= RasterizerDesc;
		PipelineDesc.BlendState							= CD3DX12_BLEND_DESC( D3D12_DEFAULT );
		PipelineDesc.DepthStencilState.DepthEnable		= TRUE;
		PipelineDesc.DepthStencilState.DepthWriteMask	= D3D12_DEPTH_WRITE_MASK_ALL;
		PipelineDesc.DepthStencilState.DepthFunc		= D3D12_COMPARISON_FUNC_GREATER;
		PipelineDesc.DepthStencilState.StencilEnable	= FALSE;
		PipelineDesc.SampleMask							= UINT_MAX;
		if (Shaders.m_VS) PipelineDesc.VS				= CD3DX12_SHADER_BYTECODE(Shaders.m_VS);
		if (Shaders.m_PS) PipelineDesc.PS				= CD3DX12_SHADER_BYTECODE(Shaders.m_PS);
		if (Shaders.m_GS) PipelineDesc.GS				= CD3DX12_SHADER_BYTECODE(Shaders.m_GS);
		if (Shaders.m_HS) PipelineDesc.HS				= CD3DX12_SHADER_BYTECODE(Shaders.m_HS);
		if (Shaders.m_DS) PipelineDesc.DS				= CD3DX12_SHADER_BYTECODE(Shaders.m_DS);
		PipelineDesc.DSVFormat							= DEPTH_FORMAT;
		PipelineDesc.NumRenderTargets					= 1;
		PipelineDesc.RTVFormats[0]						= DISPLAY_OUTPUT_FORMAT;
		PipelineDesc.SampleDesc.Count					= 1;

		Device->CreateGraphicsPipelineState( &PipelineDesc, IID_PPV_ARGS( Result->m_PipelineState.GetAddressOf() ) );
		return Result;
	}

#if D3D12_Debug_INFO
	void PipelineStateObject::SetName( const std::string& Name )
	{
		if (m_PipelineState)
		{
			m_PipelineState->SetName(StringHelper::s2ws(Name).c_str());
		}
	}
#endif

// --------------------------------------------------------------------------------------------------------------

	GraphicsPipelineState::GraphicsPipelineState(const GraphicsPipelineStateInitializer& Initializer, ID3D12RootSignature* InRootSignature)
		: PipelineStateInitializer(Initializer)
		, RootSignature(InRootSignature)
	{
		PipelineStateInitializer.BoundShaderState.AddRefResources();

		if (Initializer.BoundShaderState.m_VertexDeclaration)
			memcpy(StreamStrides, Initializer.BoundShaderState.m_VertexDeclaration->StreamStrides, sizeof(StreamStrides));
		else
			memset(StreamStrides, 0, sizeof(StreamStrides));
	}

	GraphicsPipelineState::~GraphicsPipelineState()
	{
		PipelineStateInitializer.BoundShaderState.ReleaseResources();
	}

	TRefCountPtr<GraphicsPipelineState> GraphicsPipelineState::Create( Device* InDevice, const GraphicsPipelineStateInitializer& Initializer, ID3D12RootSignature* InRootSignature )
	{
		GraphicsPipelineState* OutPipelineState = new GraphicsPipelineState(Initializer, InRootSignature);

		D3D12_GRAPHICS_PIPELINE_STATE_DESC PipelineDesc = {};
		if (VertexDeclaration* InputLayout = Initializer.BoundShaderState.m_VertexDeclaration)
		{
			PipelineDesc.InputLayout = {InputLayout->VertexElements.data(), (uint32)InputLayout->VertexElements.size()};
		}

		if (Initializer.BoundShaderState.m_HullShader && Initializer.BoundShaderState.m_DomainShader)
		{
			PipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
		}
		else
		{
			PipelineDesc.PrimitiveTopologyType = D3D12PrimitiveTypeToTopologyType(TranslatePrimitiveType(Initializer.PrimitiveType));
		}

		PipelineDesc.pRootSignature						= InRootSignature;
		PipelineDesc.RasterizerState					= Initializer.RasterizerState ? Initializer.RasterizerState->Desc : CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		PipelineDesc.BlendState							= Initializer.BlendState ? Initializer.BlendState->Desc : CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		PipelineDesc.DepthStencilState					= Initializer.DepthStencilState ? Initializer.DepthStencilState->Desc : CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		PipelineDesc.SampleMask							= UINT_MAX;
		PipelineDesc.DSVFormat							= Initializer.DepthStencilTargetFormat;
		PipelineDesc.NumRenderTargets					= Initializer.RenderTargetsEnabled;
		PipelineDesc.SampleDesc.Count					= Initializer.NumSamples;
		for (int32 i = 0; i < Initializer.RenderTargetsEnabled; i++)
		{
			PipelineDesc.RTVFormats[i] = Initializer.RenderTargetFormats[i];
		}

		if (VertexShader* VShader = Initializer.BoundShaderState.m_VertexShader)
			PipelineDesc.VS = VShader->ByteCode;

		if (HullShader* HShader = Initializer.BoundShaderState.m_HullShader)
			PipelineDesc.HS = HShader->ByteCode;

		if (DomainShader* DShader = Initializer.BoundShaderState.m_DomainShader)
			PipelineDesc.DS = DShader->ByteCode;

		if (GeometryShader* GShader = Initializer.BoundShaderState.m_GeometryShader)
			PipelineDesc.GS = GShader->ByteCode;

		if (PixelShader* PShader = Initializer.BoundShaderState.m_PixelShader)
			PipelineDesc.PS = PShader->ByteCode;

		InDevice->GetD3D12Device()->CreateGraphicsPipelineState(&PipelineDesc, IID_PPV_ARGS(OutPipelineState->PipelineState.GetInitReference()));

		return OutPipelineState;
	}

        }  // namespace Drn