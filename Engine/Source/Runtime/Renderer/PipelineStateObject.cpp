#include "DrnPCH.h"
#include "PipelineStateObject.h"

namespace Drn
{
	GraphicsPipelineState::GraphicsPipelineState(const GraphicsPipelineStateInitializer& Initializer, ID3D12RootSignature* InRootSignature)
		: PipelineStateInitializer(Initializer)
		, RootSignature(InRootSignature)
	{
		//PipelineStateInitializer.BoundShaderState.AddRefResources();

		if (Initializer.BoundShaderState.m_VertexDeclaration)
			memcpy(StreamStrides, Initializer.BoundShaderState.m_VertexDeclaration->StreamStrides, sizeof(StreamStrides));
		else
			memset(StreamStrides, 0, sizeof(StreamStrides));
	}

	GraphicsPipelineState::~GraphicsPipelineState()
	{
		//PipelineStateInitializer.BoundShaderState.ReleaseResources();
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

		PipelineDesc.pRootSignature		= InRootSignature;
		PipelineDesc.RasterizerState	= Initializer.RasterizerState ? Initializer.RasterizerState->Desc : CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		PipelineDesc.BlendState			= Initializer.BlendState ? Initializer.BlendState->Desc : CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		PipelineDesc.DepthStencilState	= Initializer.DepthStencilState ? Initializer.DepthStencilState->Desc : CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		PipelineDesc.SampleMask			= UINT_MAX;
		PipelineDesc.DSVFormat			= Initializer.DepthStencilTargetFormat;
		PipelineDesc.NumRenderTargets	= Initializer.RenderTargetsEnabled;
		PipelineDesc.SampleDesc.Count	= Initializer.NumSamples;
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

	TRefCountPtr<ComputePipelineState> ComputePipelineState::Create( Device* InDevice, ComputeShader* InComputeShader, ID3D12RootSignature* InRootSignature )
	{
		drn_check(InComputeShader);

		ComputePipelineState* OutPipelineState = new ComputePipelineState(InComputeShader);

		D3D12_COMPUTE_PIPELINE_STATE_DESC PipelineDesc = {};
		PipelineDesc.CS = InComputeShader->ByteCode;
		PipelineDesc.pRootSignature = InRootSignature;
		InDevice->GetD3D12Device()->CreateComputePipelineState(&PipelineDesc, IID_PPV_ARGS(OutPipelineState->PipelineState.GetInitReference()));

		return OutPipelineState;
	}

}