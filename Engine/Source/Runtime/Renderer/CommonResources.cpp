#include "DrnPCH.h"
#include "CommonResources.h"

LOG_DEFINE_CATEGORY( LogCommonResources, "CommonResources" );

namespace Drn
{
	CommonResources* CommonResources::m_SingletonInstance = nullptr;

	D3D12_INPUT_ELEMENT_DESC InputElement_PosUV[2] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	D3D12_INPUT_LAYOUT_DESC VertexLayout_PosUV = { InputElement_PosUV, _countof( InputElement_PosUV ) };

	CommonResources::CommonResources( ID3D12GraphicsCommandList2* CommandList )
	{
		m_ScreenTriangle = new ScreenTriangle( CommandList );
		m_ResolveAlphaBlendedPSO = new ResolveAlphaBlendedPSO(CommandList);
		m_ResolveEditorSelectionPSO = new ResolveEditorSelectionPSO(CommandList);
	}

	CommonResources::~CommonResources()
	{
		delete m_ScreenTriangle;
		delete m_ResolveAlphaBlendedPSO;
		delete m_ResolveEditorSelectionPSO;
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
		const uint32 VertexBufferSize = sizeof( TriangleVertexData );
		const uint32 IndexBufferSize = sizeof( TriangleIndexData );

		Resource* IntermediateVertexBuffer = Resource::Create(D3D12_HEAP_TYPE_UPLOAD, 
			CD3DX12_RESOURCE_DESC::Buffer( VertexBufferSize ), D3D12_RESOURCE_STATE_GENERIC_READ);

		m_VertexBuffer = Resource::Create(D3D12_HEAP_TYPE_DEFAULT, 
			CD3DX12_RESOURCE_DESC::Buffer( VertexBufferSize ), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

		Resource* IntermediateIndexBuffer = Resource::Create(D3D12_HEAP_TYPE_UPLOAD, 
			CD3DX12_RESOURCE_DESC::Buffer( IndexBufferSize ), D3D12_RESOURCE_STATE_GENERIC_READ);

		m_IndexBuffer = Resource::Create(D3D12_HEAP_TYPE_DEFAULT, 
			CD3DX12_RESOURCE_DESC::Buffer( IndexBufferSize ), D3D12_RESOURCE_STATE_INDEX_BUFFER);

#if D3D12_Debug_INFO
		IntermediateVertexBuffer->SetName( "ScreenTriangle_IntermediateVertexBuffer" );
		m_VertexBuffer->SetName( "ScreenTriangle_VertexBuffer" );
		IntermediateIndexBuffer->SetName( "ScreenTriangle_IntermediateIndexBuffer" );
		m_IndexBuffer->SetName( "ScreenTriangle_IndexBuffer"  );
#endif

		{
			UINT8*        pVertexDataBegin;
			CD3DX12_RANGE readRange( 0, 0 );
			IntermediateVertexBuffer->GetD3D12Resource()->Map( 0, &readRange, reinterpret_cast<void**>( &pVertexDataBegin ) );
			memcpy( pVertexDataBegin, &TriangleVertexData[0], VertexBufferSize );
			IntermediateVertexBuffer->GetD3D12Resource()->Unmap( 0, nullptr );

			CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
				m_VertexBuffer->GetD3D12Resource(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST );
			CommandList->ResourceBarrier(1, &barrier);

			CommandList->CopyResource(m_VertexBuffer->GetD3D12Resource(), IntermediateVertexBuffer->GetD3D12Resource());

			barrier = CD3DX12_RESOURCE_BARRIER::Transition(
				m_VertexBuffer->GetD3D12Resource(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER );
			CommandList->ResourceBarrier(1, &barrier);

			m_VertexBufferView.BufferLocation = m_VertexBuffer->GetD3D12Resource()->GetGPUVirtualAddress();
			m_VertexBufferView.StrideInBytes  = VertexBufferSize/3;
			m_VertexBufferView.SizeInBytes    = VertexBufferSize;
		}

		{
			UINT8*        pIndexDataBegin;
			CD3DX12_RANGE readRange( 0, 0 );
			IntermediateIndexBuffer->GetD3D12Resource()->Map( 0, &readRange, reinterpret_cast<void**>( &pIndexDataBegin ) );
			memcpy( pIndexDataBegin, &TriangleIndexData[0], IndexBufferSize );
			IntermediateIndexBuffer->GetD3D12Resource()->Unmap( 0, nullptr );

			CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
				m_IndexBuffer->GetD3D12Resource(), D3D12_RESOURCE_STATE_INDEX_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST );
			CommandList->ResourceBarrier(1, &barrier);

			CommandList->CopyResource(m_IndexBuffer->GetD3D12Resource(), IntermediateIndexBuffer->GetD3D12Resource());

			barrier = CD3DX12_RESOURCE_BARRIER::Transition(
				m_IndexBuffer->GetD3D12Resource(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER );
			CommandList->ResourceBarrier(1, &barrier);

			m_IndexBufferView.BufferLocation = m_IndexBuffer->GetD3D12Resource()->GetGPUVirtualAddress();
			m_IndexBufferView.Format         = DXGI_FORMAT_R32_UINT;
			m_IndexBufferView.SizeInBytes    = IndexBufferSize;
		}

		IntermediateVertexBuffer->ReleaseBufferedResource();
		IntermediateIndexBuffer->ReleaseBufferedResource();
	}

	ScreenTriangle::~ScreenTriangle()
	{
		if (m_VertexBuffer)
		{
			m_VertexBuffer->ReleaseBufferedResource();
		}

		if (m_IndexBuffer)
		{
			m_IndexBuffer->ReleaseBufferedResource();
		}
	}

// --------------------------------------------------------------------------------------

	ResolveAlphaBlendedPSO::ResolveAlphaBlendedPSO( ID3D12GraphicsCommandList2* CommandList )
	{
		m_RootSignature = nullptr;
		m_PSO = nullptr;

		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		CD3DX12_ROOT_PARAMETER1 rootParameters[2] = {};
		rootParameters[0].InitAsConstants(16, 0);
		CD3DX12_DESCRIPTOR_RANGE1 Range(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);
		rootParameters[1].InitAsDescriptorTable(1, &Range);

		CD3DX12_STATIC_SAMPLER_DESC SamplerDesc = {};
		SamplerDesc.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
		SamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		SamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		SamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		SamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		SamplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		SamplerDesc.ShaderRegister = 0;
		SamplerDesc.RegisterSpace = 0;

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription(2, rootParameters, 1, &SamplerDesc, rootSignatureFlags );

		ID3DBlob* pSerializedRootSig;
		ID3DBlob* pRootSigError;
		HRESULT Result = D3D12SerializeVersionedRootSignature(&rootSignatureDescription, &pSerializedRootSig, &pRootSigError);
		if ( FAILED(Result) )
		{
			if ( pRootSigError )
			{
				LOG(LogMaterial, Error, "shader signature serialization failed. %s", (char*)pRootSigError->GetBufferPointer());
				pRootSigError->Release();
			}

			if (pSerializedRootSig)
			{
				pSerializedRootSig->Release();
				pSerializedRootSig = nullptr;
			}

			return;
		}

		Device->CreateRootSignature(0, pSerializedRootSig->GetBufferPointer(),
			pSerializedRootSig->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature));

		std::string ShaderCode = FileSystem::ReadFileAsString( Path::ConvertProjectPath( "\\Engine\\Content\\Shader\\ResolveAlphaBlended.hlsl" ) );

		ID3DBlob* VertexShaderBlob;
		ID3DBlob* PixelShaderBlob;

		CompileShaderString( ShaderCode, "Main_VS", "vs_5_1", VertexShaderBlob);
		CompileShaderString( ShaderCode, "Main_PS", "ps_5_1", PixelShaderBlob);

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
		PipelineDesc.pRootSignature						= m_RootSignature;
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
	}

	ResolveAlphaBlendedPSO::~ResolveAlphaBlendedPSO()
	{
		m_RootSignature->Release();
		m_PSO->Release();
	}

// --------------------------------------------------------------------------------------

	ResolveEditorSelectionPSO::ResolveEditorSelectionPSO( ID3D12GraphicsCommandList2* CommandList )
	{
		m_RootSignature = nullptr;
		m_PSO = nullptr;

		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		CD3DX12_ROOT_PARAMETER1 rootParameters[2] = {};
		rootParameters[0].InitAsConstants(24, 0);
		CD3DX12_DESCRIPTOR_RANGE1 Range(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);
		rootParameters[1].InitAsDescriptorTable(1, &Range);

		CD3DX12_STATIC_SAMPLER_DESC SamplerDesc = {};
		SamplerDesc.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
		SamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		SamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		SamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		SamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		SamplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		SamplerDesc.ShaderRegister = 0;
		SamplerDesc.RegisterSpace = 0;

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription(2, rootParameters, 1, &SamplerDesc, rootSignatureFlags );

		ID3DBlob* pSerializedRootSig;
		ID3DBlob* pRootSigError;
		HRESULT Result = D3D12SerializeVersionedRootSignature(&rootSignatureDescription, &pSerializedRootSig, &pRootSigError);
		if ( FAILED(Result) )
		{
			if ( pRootSigError )
			{
				LOG(LogMaterial, Error, "shader signature serialization failed. %s", (char*)pRootSigError->GetBufferPointer());
				pRootSigError->Release();
			}

			if (pSerializedRootSig)
			{
				pSerializedRootSig->Release();
				pSerializedRootSig = nullptr;
			}

			return;
		}

		Device->CreateRootSignature(0, pSerializedRootSig->GetBufferPointer(),
			pSerializedRootSig->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature));

		std::string ShaderCode = FileSystem::ReadFileAsString( Path::ConvertProjectPath( "\\Engine\\Content\\Shader\\ResolveEditorSelection.hlsl" ) );

		ID3DBlob* VertexShaderBlob;
		ID3DBlob* PixelShaderBlob;

		CompileShaderString( ShaderCode, "Main_VS", "vs_5_1", VertexShaderBlob);
		CompileShaderString( ShaderCode, "Main_PS", "ps_5_1", PixelShaderBlob);

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
		PipelineDesc.pRootSignature						= m_RootSignature;
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
	}

	ResolveEditorSelectionPSO::~ResolveEditorSelectionPSO()
	{
		m_RootSignature->Release();
		m_PSO->Release();
	}

	void CompileShaderString( const std::string& ShaderCode, const char* EntryPoint, const char* Profile, ID3DBlob*& ShaderBlob )
	{
		UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
		flags |= D3DCOMPILE_DEBUG;
#endif

		ID3DBlob* errorBlob = nullptr;
		HRESULT hr = D3DCompile( ShaderCode.c_str(), ShaderCode.size(), NULL, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, 
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


}