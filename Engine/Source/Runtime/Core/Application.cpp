#include "DrnPCH.h"
#include "Application.h"
#include "Runtime/Renderer/Renderer.h"

#include "Editor/Editor.h"

#include <shlwapi.h>

#include <DirectXMath.h>
#include <dx12lib/Helpers.h>

using namespace DirectX;
using namespace Microsoft::WRL;

struct VertexPosColor
{
    XMFLOAT3 Position;
    XMFLOAT3 Color;
};

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


namespace Drn
{
	int Application::Run(HINSTANCE inhInstance)
	{
		m_hInstance = inhInstance;

#if defined( _DEBUG )
		dx12lib::Device::EnableDebugLayer();
#endif

		int retCode = 0;
		auto& gf = GameFramework::Create( m_hInstance );
		{
			logger = gf.CreateLogger( "ClearScreen" );

			WCHAR   path[MAX_PATH];
			HMODULE hModule = ::GetModuleHandleW( NULL );
			if ( ::GetModuleFileNameW( hModule, path, MAX_PATH ) > 0 )
			{
				::PathRemoveFileSpecW( path );
				::SetCurrentDirectoryW( path );
			}

			pDevice = dx12lib::Device::Create();

			auto description = pDevice->GetDescription();
			logger->info( L"Device Created: {}", description );

			auto& commandQueue = pDevice->GetCommandQueue( D3D12_COMMAND_LIST_TYPE_COPY );
			auto  commandList  = commandQueue.GetCommandList();

			pVertexBuffer = commandList->CopyVertexBuffer( _countof( g_Vertices ), sizeof( VertexPosColor ), g_Vertices );
			pIndexBuffer = commandList->CopyIndexBuffer( _countof( g_Indices ), DXGI_FORMAT_R16_UINT, g_Indices );
			commandQueue.ExecuteCommandList( commandList );

			pGameWindow = gf.CreateWindow( L"Clear Screen", 1920, 1080 );

			pSwapChain = pDevice->CreateSwapChain( pGameWindow->GetWindowHandle() );
			pSwapChain->SetVSync( false );

			pGameWindow->KeyPressed += KeyboardEvent::slot( &Application::OnKeyPressed, this );
			pGameWindow->Resize += ResizeEvent::slot( &Application::OnWindowResized, this );
			pGameWindow->Update += UpdateEvent::slot( &Application::OnUpdate, this );
			pGameWindow->Close += WindowCloseEvent::slot( &Application::OnWindowClose, this );

			D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
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

			pRootSignature = pDevice->CreateRootSignature( rootSignatureDescription.Desc_1_1 );

			ComPtr<ID3DBlob> vertexShaderBlob;
			ThrowIfFailed( D3DReadFileToBlob( L"TestShader_VS.cso", &vertexShaderBlob ) );

			ComPtr<ID3DBlob> pixelShaderBlob;
			ThrowIfFailed( D3DReadFileToBlob( L"TestShader_PS.cso", &pixelShaderBlob ) );

			struct PipelineStateStream
			{
				CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE        pRootSignature;
				CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT          InputLayout;
				CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY    PrimitiveTopologyType;
				CD3DX12_PIPELINE_STATE_STREAM_VS                    VS;
				CD3DX12_PIPELINE_STATE_STREAM_PS                    PS;
				CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT  DSVFormat;
				CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
			} pipelineStateStream;

			pipelineStateStream.pRootSignature        = pRootSignature->GetD3D12RootSignature().Get();
			pipelineStateStream.InputLayout           = { inputLayout, _countof( inputLayout ) };
			pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			pipelineStateStream.VS                    = CD3DX12_SHADER_BYTECODE( vertexShaderBlob.Get() );
			pipelineStateStream.PS                    = CD3DX12_SHADER_BYTECODE( pixelShaderBlob.Get() );
			pipelineStateStream.DSVFormat             = DXGI_FORMAT_D32_FLOAT;
			pipelineStateStream.RTVFormats = pSwapChain->GetRenderTarget().GetRenderTargetFormats();

			pPipelineStateObject = pDevice->CreatePipelineStateObject( pipelineStateStream );

			commandQueue.Flush();


			pGameWindow->Show();

			retCode = GameFramework::Get().Run();

			pIndexBuffer.reset();
			pVertexBuffer.reset();
			pPipelineStateObject.reset();
			pRootSignature.reset();
			pDepthTexture.reset();
			pDevice.reset();
			pSwapChain.reset();
			pGameWindow.reset();
		}

		GameFramework::Destroy();

		atexit(&dx12lib::Device::ReportLiveObjects);
		return retCode;
	}


	void Application::OnUpdate( UpdateEventArgs& e ) 
	{
		static uint64_t frameCount = 0;
		static double   totalTime  = 0.0;

		totalTime += e.DeltaTime;
		frameCount++;

		if ( totalTime > 1.0 )
		{
			auto fps   = frameCount / totalTime;
			frameCount = 0;
			totalTime -= 1.0;

			logger->info( "FPS: {:.7}", fps );

			wchar_t buffer[256];
			::swprintf_s( buffer, L"Cube [FPS: %f]", fps );
			pGameWindow->SetWindowTitle( buffer );
		}

		auto renderTarget = pSwapChain->GetRenderTarget();
		renderTarget.AttachTexture( dx12lib::AttachmentPoint::DepthStencil, pDepthTexture );

		auto viewport = renderTarget.GetViewport();

		float          angle        = static_cast<float>( e.TotalTime * 90.0 );
		const XMVECTOR rotationAxis = XMVectorSet( 0, 1, 1, 0 );
		XMMATRIX       modelMatrix  = XMMatrixRotationAxis( rotationAxis, XMConvertToRadians( angle ) );

		const XMVECTOR eyePosition = XMVectorSet( 0, 0, -10, 1 );
		const XMVECTOR focusPoint  = XMVectorSet( 0, 0, 0, 1 );
		const XMVECTOR upDirection = XMVectorSet( 0, 1, 0, 0 );
		XMMATRIX       viewMatrix  = XMMatrixLookAtLH( eyePosition, focusPoint, upDirection );

		float    aspectRatio = viewport.Width / viewport.Height;
		XMMATRIX projectionMatrix =
			XMMatrixPerspectiveFovLH( XMConvertToRadians( fieldOfView ), aspectRatio, 0.1f, 100.0f );
		XMMATRIX mvpMatrix = XMMatrixMultiply( modelMatrix, viewMatrix );
		mvpMatrix          = XMMatrixMultiply( mvpMatrix, projectionMatrix );

		auto& commandQueue = pDevice->GetCommandQueue( D3D12_COMMAND_LIST_TYPE_DIRECT );
		auto  commandList  = commandQueue.GetCommandList();

		commandList->SetPipelineState( pPipelineStateObject );
		commandList->SetGraphicsRootSignature( pRootSignature );

		commandList->SetGraphics32BitConstants( 0, mvpMatrix );

		const FLOAT clearColor[] = { 0.4f, 0.6f, 0.9f, 1.0f };
		commandList->ClearTexture( renderTarget.GetTexture( dx12lib::AttachmentPoint::Color0 ), clearColor );
		commandList->ClearDepthStencilTexture( pDepthTexture, D3D12_CLEAR_FLAG_DEPTH );

		commandList->SetRenderTarget( renderTarget );
		commandList->SetViewport( renderTarget.GetViewport() );
		commandList->SetScissorRect( CD3DX12_RECT( 0, 0, LONG_MAX, LONG_MAX ) );

		commandList->SetVertexBuffer( 0, pVertexBuffer );
		commandList->SetIndexBuffer( pIndexBuffer );
		commandList->SetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
		commandList->DrawIndexed( pIndexBuffer->GetNumIndices() );

		commandQueue.ExecuteCommandList( commandList );

		pSwapChain->Present();
	}

	void Application::OnKeyPressed( KeyEventArgs& e )
	{
		logger->info( L"KeyPressed: {}", (wchar_t)e.Char );

		switch ( e.Key )
		{
		case KeyCode::V:
				pSwapChain->ToggleVSync();
				break;
		case KeyCode::Escape:
				// Stop the application if the Escape key is pressed.
				GameFramework::Get().Stop();
				break;
		case KeyCode::Enter:
				if ( e.Alt )
				{
					[[fallthrough]];
				case KeyCode::F11:
					pGameWindow->ToggleFullscreen();
					break;
				}
		}
	}

	void Application::OnWindowResized( ResizeEventArgs& e )
	{
		logger->info( "Window Resize: {}, {}", e.Width, e.Height );
		GameFramework::Get().SetDisplaySize( e.Width, e.Height );

		pDevice->Flush();

		pSwapChain->Resize( e.Width, e.Height );

		auto depthTextureDesc = CD3DX12_RESOURCE_DESC::Tex2D( DXGI_FORMAT_D32_FLOAT, e.Width, e.Height );
		depthTextureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		D3D12_CLEAR_VALUE optimizedClearValue = {};
		optimizedClearValue.Format            = DXGI_FORMAT_D32_FLOAT;
		optimizedClearValue.DepthStencil      = { 1.0F, 0 };

		pDepthTexture = pDevice->CreateTexture( depthTextureDesc, &optimizedClearValue );
	}

	void Application::OnWindowClose( WindowCloseEventArgs& e ) 
	{
		GameFramework::Get().Stop();
	}

// 		Startup();
// 
// 		while (bRunning && !Renderer::Get()->GetMainWindow()->PendingClose())
// 		{
// 			float DeltaTime = m_Timer.ElapsedSeconds();
// 			m_Timer.Tick();
// 
// 			Tick(DeltaTime);
// 		}
// 
// 		Shutdown();
// 	}
// 
// 	void Application::Startup()
// 	{
// 		std::cout << "Start application" << std::endl;
// 
// 		Renderer::Init(m_hInstance);
// 
// #if WITH_EDITOR
// 		Editor::Get()->Init();
// #endif
// 
// 
// 	}
// 
// 	void Application::Shutdown()
// 	{
// 		std::cout << "Shutdown application" << std::endl;
// 
// 		Renderer::Get()->Shutdown();
// 	}
// 
// 	void Application::Tick(float DeltaTime)
// 	{
// 		Renderer::Get()->Tick(DeltaTime);
// 
// #if WITH_EDITOR
// 		Editor::Get()->Tick(DeltaTime);
// #endif
// 	}


}