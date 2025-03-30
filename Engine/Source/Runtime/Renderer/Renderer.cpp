#include "DrnPCH.h"
#include "Renderer.h"

#include "Runtime/Renderer/ImGui/ImGuiRenderer.h"

#include <GameFramework/Window.h>
#include <GameFramework/GameFramework.h>


LOG_DEFINE_CATEGORY( LogRenderer, "Renderer" );

using namespace DirectX;
using namespace Microsoft::WRL;

namespace Drn
{
	static VertexPosColor g_Vertices[8] = {
		{ XMFLOAT3( -1.0f, -1.0f, -1.0f ), XMFLOAT3( 0.0f, 0.0f, 0.0f ) },  // 0
		{ XMFLOAT3( -1.0f, 1.0f, -1.0f ), XMFLOAT3( 0.0f, 1.0f, 0.0f ) },   // 1
		{ XMFLOAT3( 1.0f, 1.0f, -1.0f ), XMFLOAT3( 1.0f, 1.0f, 0.0f ) },    // 2
		{ XMFLOAT3( 1.0f, -1.0f, -1.0f ), XMFLOAT3( 1.0f, 0.0f, 0.0f ) },   // 3
		{ XMFLOAT3( -1.0f, -1.0f, 1.0f ), XMFLOAT3( 0.0f, 0.0f, 1.0f ) },   // 4
		{ XMFLOAT3( -1.0f, 1.0f, 1.0f ), XMFLOAT3( 0.0f, 1.0f, 1.0f ) },    // 5
		{ XMFLOAT3( 1.0f, 1.0f, 1.0f ), XMFLOAT3( 1.0f, 1.0f, 1.0f ) },     // 6
		{ XMFLOAT3( 1.0f, -1.0f, 1.0f ), XMFLOAT3( 1.0f, 0.0f, 1.0f ) }     // 7
	};

	static WORD g_Indices[36] = { 0, 1, 2, 0, 2, 3, 4, 6, 5, 4, 7, 6, 4, 5, 1, 4, 1, 0,
									3, 2, 6, 3, 6, 7, 1, 5, 6, 1, 6, 2, 4, 0, 3, 4, 3, 7 };

	Renderer* Renderer::SingletonInstance;

	Renderer::Renderer() 
	{
		
	}

	Renderer* Renderer::Get()
	{
		return SingletonInstance;
	}

	void Renderer::Init( HINSTANCE inhInstance, Window* InMainWindow )
	{
		SingletonInstance = new Renderer();

		SingletonInstance->m_MainWindow = InMainWindow;
		SingletonInstance->Init_Internal();
	}

	void Renderer::Init_Internal() 
	{
		m_Device = dx12lib::Device::Create();

		auto        description = m_Device->GetDescription();
		std::string description_str( StringHelper::ws2s(description) );
		LOG( LogRenderer, Info, "%s", description_str.c_str() );

		auto& commandQueue = m_Device->GetCommandQueue( D3D12_COMMAND_LIST_TYPE_COPY );
		m_CommandList = commandQueue.GetCommandList();

// -------------------------------------------------------------------------------

		AssetManager::Init();

		CubeStaticMeshAsset = AssetHandle<StaticMesh>(Path::ConvertFullPath("Test.drn"));
		CubeStaticMeshAsset.Load();
		CubeStaticMeshAsset.Get()->UploadResources(m_CommandList.get());

// -------------------------------------------------------------------------------

		m_VertexBuffer = m_CommandList->CopyVertexBuffer( _countof( g_Vertices ), sizeof( VertexPosColor ), g_Vertices );
		m_IndexBuffer = m_CommandList->CopyIndexBuffer( _countof( g_Indices ), DXGI_FORMAT_R16_UINT, g_Indices );
		commandQueue.ExecuteCommandList( m_CommandList );

		m_SwapChain = m_Device->CreateSwapChain( m_MainWindow->GetWindowHandle(), DXGI_FORMAT_R8G8B8A8_UNORM );
		m_SwapChain->SetVSync( false );

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
		pipelineStateStream.SampleDesc = sampleDesc;

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

		commandQueue.Flush();

#if WITH_EDITOR
		ImGuiRenderer::Get()->Init();
#endif
	}

	void Renderer::Shutdown()
	{
		LOG(LogRenderer, Info, "Renderer shutdown.");

#if WITH_EDITOR
		ImGuiRenderer::Get()->Shutdown();
#endif

		SingletonInstance->CubeStaticMeshAsset.Release();

		AssetManager::Shutdown();


		SingletonInstance->m_CommandList.reset();
		SingletonInstance->m_IndexBuffer.reset();
		SingletonInstance->m_VertexBuffer.reset();
		SingletonInstance->m_PipelineStateObject.reset();
		SingletonInstance->m_RootSignature.reset();
		SingletonInstance->m_DepthTexture.reset();
		SingletonInstance->m_RenderTarget.Reset();
		SingletonInstance->m_SwapChain.reset();
		SingletonInstance->m_Device.reset();

		delete SingletonInstance;
		SingletonInstance = nullptr;
	}

	void Renderer::ToggleSwapChain() 
	{
		m_SwapChain->ToggleVSync();
	}

	void Renderer::MainWindowResized( float InWidth, float InHeight ) 
	{
		m_Device->Flush();

		m_SwapChain->Resize( InWidth, InHeight );

#ifndef WITH_EDITOR
		ViewportResized(InWidth, InHeight);
#endif
	}

	void Renderer::ViewportResized( float InWidth, float InHeight ) 
	{
		LOG( LogRenderer, Info, "viewport resize: %ix%i", (int)InWidth, (int)InHeight);

		m_Device->Flush();
		m_RenderTarget.Resize( InWidth, InHeight );
	}

	ID3D12Resource* Renderer::GetViewportResource() 
	{
		return m_RenderTarget.GetTexture(dx12lib::AttachmentPoint::Color0)->GetD3D12Resource().Get();
	}

	void Renderer::Tick( float DeltaTime )
	{
		// @TODO: move time to accessible location
		TotalTime += DeltaTime;

		float          angle        = static_cast<float>( TotalTime * 90.0 );
		const XMVECTOR rotationAxis = XMVectorSet( 0, 1, 1, 0 );
		XMMATRIX       modelMatrix  = XMMatrixRotationAxis( rotationAxis, XMConvertToRadians( angle ) );

		const XMVECTOR eyePosition = XMVectorSet( 0, 0, -10, 1 );
		const XMVECTOR focusPoint  = XMVectorSet( 0, 0, 0, 1 );
		const XMVECTOR upDirection = XMVectorSet( 0, 1, 0, 0 );
		XMMATRIX       viewMatrix  = XMMatrixLookAtLH( eyePosition, focusPoint, upDirection );

		auto viewport = m_RenderTarget.GetViewport();

		float    aspectRatio = viewport.Width / viewport.Height;
		XMMATRIX projectionMatrix =
			XMMatrixPerspectiveFovLH( XMConvertToRadians( m_fieldOfView ), aspectRatio, 0.1f, 100.0f );
		XMMATRIX mvpMatrix = XMMatrixMultiply( modelMatrix, viewMatrix );
		mvpMatrix          = XMMatrixMultiply( mvpMatrix, projectionMatrix );

		auto& commandQueue = m_Device->GetCommandQueue( D3D12_COMMAND_LIST_TYPE_DIRECT );
		m_CommandList  = commandQueue.GetCommandList();

		FLOAT clearColor[] = { 0.4f, 0.6f, 0.9f, 1.0f };
		{

			m_CommandList->ClearTexture( m_RenderTarget.GetTexture( dx12lib::AttachmentPoint::Color0 ),
										clearColor );
			m_CommandList->ClearDepthStencilTexture(
				m_RenderTarget.GetTexture( dx12lib::AttachmentPoint::DepthStencil ), D3D12_CLEAR_FLAG_DEPTH );
		}

		m_CommandList->SetPipelineState( m_PipelineStateObject );
		m_CommandList->SetGraphicsRootSignature( m_RootSignature );

		m_CommandList->SetGraphics32BitConstants( 0, mvpMatrix );

		m_CommandList->SetRenderTarget( m_RenderTarget );
		m_CommandList->SetViewport( m_RenderTarget.GetViewport() );
		m_CommandList->SetScissorRect( CD3DX12_RECT( 0, 0, LONG_MAX, LONG_MAX ) );

		m_CommandList->SetVertexBuffer( 0, m_VertexBuffer );
		m_CommandList->SetIndexBuffer( m_IndexBuffer );

		m_CommandList->SetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
		m_CommandList->DrawIndexed( m_IndexBuffer->GetNumIndices() );

		auto& swapChainRT         = m_SwapChain->GetRenderTarget();
		auto  swapChainBackBuffer = swapChainRT.GetTexture( dx12lib::AttachmentPoint::Color0 );
		auto  msaaRenderTarget    = m_RenderTarget.GetTexture( dx12lib::AttachmentPoint::Color0 );

		for (Scene* S : AllocatedScenes)
		{
			S->Render(m_CommandList.get());
		}

		m_CommandList->SetRenderTarget( swapChainRT );
		m_CommandList->ClearTexture( swapChainRT.GetTexture( dx12lib::AttachmentPoint::Color0 ), clearColor );

#if WITH_EDITOR
		ImGuiRenderer::Get()->Tick( 1, swapChainBackBuffer->GetRenderTargetView(),
									m_CommandList->GetD3D12CommandList().Get() );
#else
		// m_CommandList->ResolveSubresource( swapChainBackBuffer, msaaRenderTarget );
		commandList->CopyResource( swapChainBackBuffer, msaaRenderTarget );

#endif

		commandQueue.ExecuteCommandList( m_CommandList );

#if WITH_EDITOR
		ImGuiRenderer::Get()->PostExecuteCommands();
#endif

		m_SwapChain->Present();
	}

	Scene* Renderer::AllocateScene( World* InWorld )
	{
		Scene* NewScene = new Scene(InWorld);
		AllocatedScenes.insert(NewScene);

		return NewScene;
	}

	void Renderer::RemoveScene( Scene* InScene )
	{
		AllocatedScenes.erase(InScene);
	}

	void Renderer::RemoveAndInvalidateScene( Scene*& InScene )
	{
		RemoveScene(InScene);
		delete InScene;
		InScene = nullptr;
	}

}