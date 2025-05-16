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


	PipelineStateObject* PipelineStateObject::CreateMainPassPSO(ID3D12RootSignature* RootSignature, D3D12_CULL_MODE CullMode, EInputLayoutType InputLayoutType,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE PrimitiveType, ID3DBlob* VS, ID3DBlob* PS, ID3DBlob* GS, ID3DBlob* DS, ID3DBlob* HS)
	{
		PipelineStateObject* Result = new PipelineStateObject();

		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		DXGI_FORMAT backBufferFormat  = DXGI_FORMAT_R8G8B8A8_UNORM;
		DXGI_FORMAT depthBufferFormat = DXGI_FORMAT_D32_FLOAT;

		D3D12_RASTERIZER_DESC RasterizerDesc = CD3DX12_RASTERIZER_DESC( D3D12_DEFAULT );
		RasterizerDesc.CullMode = CullMode;

		D3D12_GRAPHICS_PIPELINE_STATE_DESC PipelineDesc = {};
		PipelineDesc.pRootSignature						= RootSignature;
		PipelineDesc.InputLayout						= InputLayout::GetLayoutDescriptionForType(InputLayoutType);
		PipelineDesc.PrimitiveTopologyType				= PrimitiveType;
		PipelineDesc.RasterizerState					= RasterizerDesc;
		PipelineDesc.BlendState							= CD3DX12_BLEND_DESC( D3D12_DEFAULT );
		PipelineDesc.DepthStencilState.DepthEnable		= TRUE;
		PipelineDesc.DepthStencilState.DepthWriteMask	= D3D12_DEPTH_WRITE_MASK_ALL;
		PipelineDesc.DepthStencilState.DepthFunc		= D3D12_COMPARISON_FUNC_GREATER;
		PipelineDesc.DepthStencilState.StencilEnable	= FALSE;
		PipelineDesc.SampleMask							= UINT_MAX;
		if (VS) PipelineDesc.VS							= CD3DX12_SHADER_BYTECODE(VS);
		if (PS) PipelineDesc.PS							= CD3DX12_SHADER_BYTECODE(PS);
		if (GS) PipelineDesc.GS							= CD3DX12_SHADER_BYTECODE(GS);
		if (HS) PipelineDesc.HS							= CD3DX12_SHADER_BYTECODE(HS);
		if (DS) PipelineDesc.DS							= CD3DX12_SHADER_BYTECODE(DS);
		PipelineDesc.DSVFormat							= depthBufferFormat;
		PipelineDesc.NumRenderTargets					= 2;
		PipelineDesc.RTVFormats[0]						= backBufferFormat;
		PipelineDesc.RTVFormats[1]						= GBUFFER_GUID_FORMAT;
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