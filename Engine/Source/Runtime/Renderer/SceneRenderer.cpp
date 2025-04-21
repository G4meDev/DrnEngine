#include "DrnPCH.h"
#include "SceneRenderer.h"

LOG_DEFINE_CATEGORY( LogSceneRenderer, "SceneRenderer" );

using namespace DirectX;
using namespace Microsoft::WRL;

namespace Drn
{
	SceneRenderer::SceneRenderer(Scene* InScene)
		: m_Scene(InScene)
        , m_RenderingEnabled(true)
	{
		Init(Renderer::Get()->m_CommandList.get());
	}

	SceneRenderer::~SceneRenderer()
	{
#if WITH_EDITOR
		m_DebugViewPhysic.Shutdown();
#endif

		m_PipelineStateObject.reset();
		m_RootSignature.reset();
		m_DepthTexture.reset();
		m_RenderTarget.Reset();
	}

	void SceneRenderer::Init(dx12lib::CommandList* CommandList)
	{
		m_Device = Renderer::Get()->GetDevice();
		auto& commandQueue = m_Device->GetCommandQueue( D3D12_COMMAND_LIST_TYPE_COPY );

		D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};

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

		m_RootSignature = m_Device->CreateRootSignature( rootSignatureDescription.Desc_1_1 );

		ComPtr<ID3DBlob> vertexShaderBlob;
		ThrowIfFailed( D3DReadFileToBlob( L"TestShader_VS.cso", &vertexShaderBlob ) );

		ComPtr<ID3DBlob> pixelShaderBlob;
		ThrowIfFailed( D3DReadFileToBlob( L"TestShader_PS.cso", &pixelShaderBlob ) );

		DXGI_FORMAT backBufferFormat  = DXGI_FORMAT_R8G8B8A8_UNORM;
		DXGI_FORMAT depthBufferFormat = DXGI_FORMAT_D32_FLOAT;

		// DXGI_SAMPLE_DESC sampleDesc = pDevice->GetMultisampleQualityLevels( backBufferFormat );

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
		pipelineStateStream.InputLayout           = { inputLayout, _countof( inputLayout ) };
		pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		pipelineStateStream.VS                    = CD3DX12_SHADER_BYTECODE( vertexShaderBlob.Get() );
		pipelineStateStream.PS                    = CD3DX12_SHADER_BYTECODE( pixelShaderBlob.Get() );
		pipelineStateStream.DSVFormat             = depthBufferFormat;
		pipelineStateStream.RTVFormats            = rtvFormats;
		pipelineStateStream.SampleDesc            = sampleDesc;

		m_PipelineStateObject = m_Device->CreatePipelineStateObject( pipelineStateStream );

		auto colorDesc =
			CD3DX12_RESOURCE_DESC::Tex2D( backBufferFormat, 1920, 1080, 1, 1, sampleDesc.Count,
											sampleDesc.Quality, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET );
		D3D12_CLEAR_VALUE colorClearValue;
		colorClearValue.Format   = colorDesc.Format;
		colorClearValue.Color[0] = 0.4f;
		colorClearValue.Color[1] = 0.6f;
		colorClearValue.Color[2] = 0.9f;
		colorClearValue.Color[3] = 1.0f;

		auto colorTexture = m_Device->CreateTexture( colorDesc, &colorClearValue );
		colorTexture->SetName( L"Color Render Target" );

		// Create a depth buffer.
		auto depthDesc =
			CD3DX12_RESOURCE_DESC::Tex2D( depthBufferFormat, 1920, 1080, 1, 1, sampleDesc.Count,
											sampleDesc.Quality, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL );
		D3D12_CLEAR_VALUE depthClearValue;
		depthClearValue.Format       = depthDesc.Format;
		depthClearValue.DepthStencil = { 1.0f, 0 };

		auto depthTexture = m_Device->CreateTexture( depthDesc, &depthClearValue );
		depthTexture->SetName( L"Depth Render Target" );

		m_RenderTarget.AttachTexture( dx12lib::AttachmentPoint::Color0, colorTexture );
		m_RenderTarget.AttachTexture( dx12lib::AttachmentPoint::DepthStencil, depthTexture );

		m_DebugViewPhysic.Init(this, CommandList);

		//commandQueue.Flush();
	}

	void SceneRenderer::BeginRender(dx12lib::CommandList* CommandList)
	{
/*
		float          angle        = static_cast<float>( Renderer::Get()->TotalTime * 90.0 );
		const XMVECTOR rotationAxis = XMVectorSet( 0, 1, 1, 0 );
		XMMATRIX       modelMatrix  = XMMatrixRotationAxis( rotationAxis, XMConvertToRadians( angle ) );

		auto viewport = m_RenderTarget.GetViewport();
		float    aspectRatio = viewport.Width / viewport.Height;
		
		XMMATRIX viewMatrix;
		XMMATRIX projectionMatrix;
		
		TargetCamera->CalculateMatrices(viewMatrix, projectionMatrix, aspectRatio);
		
		XMMATRIX mvpMatrix = XMMatrixMultiply( modelMatrix, viewMatrix );
		mvpMatrix          = XMMatrixMultiply( mvpMatrix, projectionMatrix );
*/

		auto& commandQueue = m_Device->GetCommandQueue( D3D12_COMMAND_LIST_TYPE_DIRECT );

		FLOAT clearColor[] = { 0.4f, 0.6f, 0.9f, 1.0f };
		{
				CommandList->ClearTexture( m_RenderTarget.GetTexture( dx12lib::AttachmentPoint::Color0 ),
											clearColor );
				CommandList->ClearDepthStencilTexture(
					m_RenderTarget.GetTexture( dx12lib::AttachmentPoint::DepthStencil ),
					D3D12_CLEAR_FLAG_DEPTH );
		}

		CommandList->SetPipelineState( m_PipelineStateObject );
		CommandList->SetGraphicsRootSignature( m_RootSignature );

//		CommandList->SetGraphics32BitConstants( 0, mvpMatrix );

		CommandList->SetRenderTarget( m_RenderTarget );
		CommandList->SetViewport( m_RenderTarget.GetViewport() );
		CommandList->SetScissorRect( CD3DX12_RECT( 0, 0, LONG_MAX, LONG_MAX ) );
		
		CommandList->SetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
	}

	void SceneRenderer::RenderBasePass(dx12lib::CommandList* CommandList)
	{
		CommandList->SetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

		for (StaticMeshComponent* Mesh : m_Scene->m_StaticMeshComponents)
		{
			if (!Mesh->GetMesh())
			{
				continue;
			}

			if (!Mesh->GetMesh()->m_LoadedOnGPU)
			{
				Mesh->GetMesh()->UploadResources(CommandList);
			}

			XMMATRIX modelMatrix = Matrix(Mesh->GetWorldTransform()).Get();

			auto viewport = m_RenderTarget.GetViewport();
			float    aspectRatio = viewport.Width / viewport.Height;
		
			XMMATRIX viewMatrix;
			XMMATRIX projectionMatrix;
		
			m_CameraActor->GetCameraComponent()->CalculateMatrices(viewMatrix, projectionMatrix, aspectRatio);
		
			XMMATRIX mvpMatrix = XMMatrixMultiply( modelMatrix, viewMatrix );
			mvpMatrix          = XMMatrixMultiply( mvpMatrix, projectionMatrix );

			CommandList->SetGraphics32BitConstants( 0, mvpMatrix );

			for (const StaticMeshRenderProxy& RenderProxy : Mesh->GetMesh()->RenderProxies)
			{
				CommandList->SetVertexBuffer( 0, RenderProxy.VertexBuffer );
				CommandList->SetIndexBuffer( RenderProxy.IndexBuffer );
				CommandList->DrawIndexed( RenderProxy.IndexBuffer->GetNumIndices() );
			}
		}

		for (PrimitiveSceneProxy* Proxy : m_Scene->m_PrimitiveProxies)
		{
			Proxy->RenderMainPass(CommandList, this);
		}
	}

	void SceneRenderer::Render( dx12lib::CommandList* CommandList )
	{
		if (!m_RenderingEnabled)
		{
			return;
		}

		BeginRender(CommandList);
		RenderBasePass(CommandList);

#if WITH_EDITOR
		//m_DebugViewPhysic.RenderCollisions(CommandList);
		//m_DebugViewPhysic.RenderPhysxDebug(CommandList);
#endif
	}

	ID3D12Resource* SceneRenderer::GetViewResource()
	{
		return m_RenderTarget.GetTexture( dx12lib::AttachmentPoint::Color0 )->GetD3D12Resource().Get();
	}

	void SceneRenderer::ResizeView( const IntPoint& InSize )
	{
		//LOG( LogRenderer, Info, "viewport resize: %ix%i", (int)InWidth, (int)InHeight );

		m_Device->Flush();
		m_RenderTarget.Resize( InSize.X, InSize.Y);
	}

	void SceneRenderer::SetRenderingEnabled( bool Enabled )
	{
		m_RenderingEnabled = Enabled;
	}

}