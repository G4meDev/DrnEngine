#include "DrnPCH.h"
#include "LineBatchComponent.h"

using namespace Microsoft::WRL;

namespace Drn
{
	void LineBatchComponent::TickComponent( float DeltaTime )
	{
		bool Dirty = false;

		for ( auto it = m_Lines.begin(); it != m_Lines.end(); )
		{
			BatchLine& Line = *it;

			// at least draw for single frame
			if ( Line.RemainingLifetime < 0 )
			{
				it = m_Lines.erase(it);
				Dirty = true;
			}
			
			else
			{
				Line.RemainingLifetime -= DeltaTime;
				it++;
			}
		}

		if (Dirty)
		{
			MarkRenderStateDirty();
		}
	}

	void LineBatchComponent::RegisterComponent( World* InOwningWorld )
	{
		PrimitiveComponent::RegisterComponent(InOwningWorld);

		m_SceneProxy = new LineBatchSceneProxy(this);
		InOwningWorld->GetScene()->AddPrimitiveProxy(m_SceneProxy);
	}

	void LineBatchComponent::UnRegisterComponent()
	{
		if (GetWorld()->GetScene())
		{
			GetWorld()->GetScene()->RemovePrimitiveProxy(m_SceneProxy);
		}

		PrimitiveComponent::UnRegisterComponent();
	}

	void LineBatchComponent::DrawLine( const Vector& Start, const Vector& End, const Vector& Color, float Lifetime )
	{
		m_Lines.push_back(BatchLine(Start, End, Color, Lifetime));
		MarkRenderStateDirty();
	}

	void LineBatchComponent::DrawLines( const std::vector<BatchLine>& InLines )
	{
		m_Lines.insert(std::end(m_Lines), std::begin(InLines), std::end(InLines));
		MarkRenderStateDirty();
	}

	void LineBatchComponent::Flush()
	{
		m_Lines.clear();
		MarkRenderStateDirty();
	}

// --------------------------------------------------------------------------------------------------------------------------------

	LineBatchSceneProxy::LineBatchSceneProxy( LineBatchComponent* InLineBatchComponent )
		: PrimitiveSceneProxy(InLineBatchComponent)
		, m_LineComponent(InLineBatchComponent)
	{
		
	}

	LineBatchSceneProxy::~LineBatchSceneProxy()
	{
		
	}

	void LineBatchSceneProxy::RenderMainPass( dx12lib::CommandList* CommandList, SceneRenderer* Renderer ) const
	{
		if (!m_HasValidData)
		{
			return;
		}

		CommandList->SetPipelineState(m_PipelineStateObject);
		CommandList->SetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_LINELIST );

		XMMATRIX modelMatrix = Matrix().Get();

		auto viewport = Renderer->m_RenderTarget.GetViewport();
		float    aspectRatio = viewport.Width / viewport.Height;
		
		XMMATRIX viewMatrix;
		XMMATRIX projectionMatrix;
		
		Renderer->m_CameraActor->GetCameraComponent()->CalculateMatrices(viewMatrix, projectionMatrix, aspectRatio);
		
		XMMATRIX mvpMatrix = XMMatrixMultiply( modelMatrix, viewMatrix );
		mvpMatrix          = XMMatrixMultiply( mvpMatrix, projectionMatrix );

		CommandList->SetGraphics32BitConstants( 0, mvpMatrix );

		CommandList->SetVertexBuffer( 0, m_VertexBuffer );
		CommandList->SetIndexBuffer( m_IndexBuffer );
		CommandList->DrawIndexed( m_IndexBuffer->GetNumIndices());
	}

	void LineBatchSceneProxy::UpdateResources(dx12lib::CommandList* CommandList)
	{
		D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

		CD3DX12_ROOT_PARAMETER1 rootParameters[1];
		rootParameters[0].InitAsConstants( sizeof( XMMATRIX ) / 4, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX );

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription(
			_countof( rootParameters ), rootParameters, 0, nullptr, rootSignatureFlags );

		m_RootSignature = CommandList->GetDevice().CreateRootSignature( rootSignatureDescription.Desc_1_1 );

		ComPtr<ID3DBlob> vertexShaderBlob;
		ThrowIfFailed( D3DReadFileToBlob( L"TestShader_VS.cso", &vertexShaderBlob ) );

		ComPtr<ID3DBlob> pixelShaderBlob;
		ThrowIfFailed( D3DReadFileToBlob( L"TestShader_PS.cso", &pixelShaderBlob ) );

		DXGI_FORMAT backBufferFormat  = DXGI_FORMAT_R8G8B8A8_UNORM;
		DXGI_FORMAT depthBufferFormat = DXGI_FORMAT_D32_FLOAT;

		DXGI_SAMPLE_DESC sampleDesc = {};
		sampleDesc.Count            = 1;

		D3D12_RT_FORMAT_ARRAY rtvFormats = {};
		rtvFormats.NumRenderTargets      = 1;
		rtvFormats.RTFormats[0]          = backBufferFormat;

		struct PipelineStateStream
		{
			CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE        pRootSignature;
			CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT          InputLayout;
			CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY    PrimitiveTopologyType;
			CD3DX12_PIPELINE_STATE_STREAM_VS                    VS;
			CD3DX12_PIPELINE_STATE_STREAM_PS                    PS;
			CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT  DSVFormat;
			CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
			CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_DESC           SampleDesc;
		} pipelineStateStream;

		pipelineStateStream.pRootSignature        = m_RootSignature->GetD3D12RootSignature().Get();
		pipelineStateStream.InputLayout           = { VertexLayout::Color, _countof( VertexLayout::Color) };
		pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
		pipelineStateStream.VS                    = CD3DX12_SHADER_BYTECODE( vertexShaderBlob.Get() );
		pipelineStateStream.PS                    = CD3DX12_SHADER_BYTECODE( pixelShaderBlob.Get() );
		pipelineStateStream.DSVFormat             = depthBufferFormat;
		pipelineStateStream.RTVFormats            = rtvFormats;
		pipelineStateStream.SampleDesc            = sampleDesc;

		m_PipelineStateObject = CommandList->GetDevice().CreatePipelineStateObject(pipelineStateStream);

		UpdateBuffers(CommandList);
	}

	void LineBatchSceneProxy::UpdateBuffers( dx12lib::CommandList* CommandList )
	{
		m_VertexData.clear();
		m_VertexData.reserve(m_LineComponent->m_Lines.size() * 2);

		m_IndexData.clear();
		m_IndexData.reserve(m_LineComponent->m_Lines.size() * 2);

		for (uint32 i = 0; i < m_LineComponent->m_Lines.size(); i++)
		{
			const BatchLine& Line = m_LineComponent->m_Lines[i];

			m_VertexData.push_back(VertexData_Color(Line.Start, Line.Color));
			m_VertexData.push_back(VertexData_Color(Line.End, Line.Color));

			m_IndexData.push_back( i * 2 );
			m_IndexData.push_back( i * 2 + 1 );
		}

		if (m_VertexData.size() > 0 && m_IndexData.size() > 0)
		{
			m_VertexBuffer = CommandList->CopyVertexBuffer( m_VertexData.size(), sizeof(VertexData_Color), m_VertexData.data());
			m_IndexBuffer = CommandList->CopyIndexBuffer( m_IndexData.size(), DXGI_FORMAT_R32_UINT, m_IndexData.data());

			m_HasValidData = true;
		}

		else
		{
			m_HasValidData = false;
		}
	}

}