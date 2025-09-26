#include "DrnPCH.h"
#include "PipelineStateObject.h"

namespace Drn
{
	PipelineStateObject::PipelineStateObject()
		: BufferedResource()
		, m_PipelineState(nullptr)
	{
	}

	PipelineStateObject::~PipelineStateObject()
	{
	}


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
		PipelineDesc.NumRenderTargets					= 5;
		PipelineDesc.RTVFormats[0]						= GBUFFER_COLOR_DEFERRED_FORMAT;
		PipelineDesc.RTVFormats[1]						= GBUFFER_BASE_COLOR_FORMAT;
		PipelineDesc.RTVFormats[2]						= GBUFFER_WORLD_NORMAL_FORMAT;
		PipelineDesc.RTVFormats[3]						= GBUFFER_MASKS_FORMAT;
		PipelineDesc.RTVFormats[4]						= GBUFFER_VELOCITY_FORMAT;
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

}