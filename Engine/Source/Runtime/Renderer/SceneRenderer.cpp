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
		m_DepthTexture.reset();
		m_RenderTarget.Reset();
	}

	void SceneRenderer::Init(dx12lib::CommandList* CommandList)
	{
		m_Device = Renderer::Get()->GetDevice();
		auto& commandQueue = m_Device->GetCommandQueue( D3D12_COMMAND_LIST_TYPE_COPY );

		DXGI_FORMAT backBufferFormat  = DXGI_FORMAT_R8G8B8A8_UNORM;
		DXGI_FORMAT depthBufferFormat = DXGI_FORMAT_D32_FLOAT;

		DXGI_SAMPLE_DESC sampleDesc = {};
		sampleDesc.Count            = 1;

		D3D12_RT_FORMAT_ARRAY rtvFormats = {};
		rtvFormats.NumRenderTargets      = 1;
		rtvFormats.RTFormats[0]          = backBufferFormat;

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

		//commandQueue.Flush();
	}

	void SceneRenderer::BeginRender(dx12lib::CommandList* CommandList)
	{
		SCOPE_STAT( BeginRender );

		auto& commandQueue = m_Device->GetCommandQueue( D3D12_COMMAND_LIST_TYPE_DIRECT );

		FLOAT clearColor[] = { 0.4f, 0.6f, 0.9f, 1.0f };
		{
			CommandList->ClearTexture( m_RenderTarget.GetTexture( dx12lib::AttachmentPoint::Color0 ),
										clearColor );
			CommandList->ClearDepthStencilTexture(
				m_RenderTarget.GetTexture( dx12lib::AttachmentPoint::DepthStencil ),
				D3D12_CLEAR_FLAG_DEPTH );
		}

		CommandList->SetRenderTarget( m_RenderTarget );
		CommandList->SetViewport( m_RenderTarget.GetViewport() );
		CommandList->SetScissorRect( CD3DX12_RECT( 0, 0, LONG_MAX, LONG_MAX ) );
	}

	void SceneRenderer::RenderBasePass(dx12lib::CommandList* CommandList)
	{
		SCOPE_STAT( RenderBasePass );

		for (PrimitiveSceneProxy* Proxy : m_Scene->m_PrimitiveProxies)
		{
			Proxy->RenderMainPass(CommandList, this);
		}
	}

	void SceneRenderer::Render( dx12lib::CommandList* CommandList )
	{
		SCOPE_STAT(SceneRendererRender);

		if (!m_RenderingEnabled)
		{
			return;
		}

		BeginRender(CommandList);
		RenderBasePass(CommandList);
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