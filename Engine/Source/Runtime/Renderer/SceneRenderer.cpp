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
		, m_RenderSize(1920, 1080)
		, m_ScissorRect(CD3DX12_RECT( 0, 0, LONG_MAX, LONG_MAX ))
	{
		Init(Renderer::Get()->GetCommandList());
	}

	SceneRenderer::~SceneRenderer()
	{
	}

	void SceneRenderer::Init(ID3D12GraphicsCommandList2* CommandList)
	{
		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
		dsvHeapDesc.NumDescriptors = 1;
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		Device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_DSVHeap));

		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = 1;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		Device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_RTVHeap));

		ResizeView(IntPoint(1920, 1080));
	}

	void SceneRenderer::BeginRender(ID3D12GraphicsCommandList2* CommandList)
	{
		SCOPE_STAT( BeginRender );

		FLOAT clearColor[] = { 0.4f, 0.6f, 0.9f, 1.0f };
		CommandList->ClearDepthStencilView(m_DSVHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1, 0, 0, nullptr);
		CommandList->ClearRenderTargetView(m_RTVHeap->GetCPUDescriptorHandleForHeapStart(), clearColor, 0, nullptr);

		CommandList->OMSetRenderTargets(1, &m_RTVHeap->GetCPUDescriptorHandleForHeapStart(), false, &m_DSVHeap->GetCPUDescriptorHandleForHeapStart());
		CommandList->RSSetViewports(1, &m_Viewport);
		CommandList->RSSetScissorRects(1, &m_ScissorRect);
	}

	void SceneRenderer::RenderBasePass( ID3D12GraphicsCommandList2* CommandList )
	{
		SCOPE_STAT( RenderBasePass );

		for (PrimitiveSceneProxy* Proxy : m_Scene->m_PrimitiveProxies)
		{
			Proxy->RenderMainPass(CommandList, this);
		}
	}

	void SceneRenderer::Render( ID3D12GraphicsCommandList2* CommandList )
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
		return m_ColorTarget.Get();
	}

	void SceneRenderer::ResizeView( const IntPoint& InSize )
	{
		Renderer::Get()->Flush();
		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		m_RenderSize = IntPoint::ComponentWiseMax(InSize, IntPoint(1));
		m_Viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(m_RenderSize.X), static_cast<float>(m_RenderSize.Y));
		D3D12_CLEAR_VALUE optimizedClearValue = {};
		optimizedClearValue.Format = DEPTH_FORMAT;
		optimizedClearValue.DepthStencil = { 1.0f, 0 };

		Device->CreateCommittedResource( &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Tex2D(DEPTH_FORMAT, m_RenderSize.X, m_RenderSize.Y,
			1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL), D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&optimizedClearValue, IID_PPV_ARGS(m_DepthTarget.ReleaseAndGetAddressOf()) );

		D3D12_DEPTH_STENCIL_VIEW_DESC dsv = {};
		dsv.Format = DEPTH_FORMAT;
		dsv.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsv.Texture2D.MipSlice = 0;
		dsv.Flags = D3D12_DSV_FLAG_NONE;

		Device->CreateDepthStencilView(m_DepthTarget.Get(), &dsv, m_DSVHeap->GetCPUDescriptorHandleForHeapStart());

#if D3D12_Debug_INFO
		m_DepthTarget->SetName( L"Depth Render Target" );
#endif

// ----------------------------------------------------------------------------------------------------------------------------

		D3D12_CLEAR_VALUE colorClearValue;
		colorClearValue.Format   = DISPLAY_OUTPUT_FORMAT;
		colorClearValue.Color[0] = 0.4f;
		colorClearValue.Color[1] = 0.6f;
		colorClearValue.Color[2] = 0.9f;
		colorClearValue.Color[3] = 1.0f;

		Device->CreateCommittedResource( &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Tex2D(DISPLAY_OUTPUT_FORMAT, m_RenderSize.X, m_RenderSize.Y,
			1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET), D3D12_RESOURCE_STATE_RENDER_TARGET,
			&colorClearValue, IID_PPV_ARGS(m_ColorTarget.ReleaseAndGetAddressOf()) );

		D3D12_RENDER_TARGET_VIEW_DESC rtv = {};
		rtv.Format = DISPLAY_OUTPUT_FORMAT;
		rtv.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		rtv.Texture2D.MipSlice = 0;

		Device->CreateRenderTargetView(m_ColorTarget.Get(), &rtv, m_RTVHeap->GetCPUDescriptorHandleForHeapStart());

#if D3D12_Debug_INFO
		m_ColorTarget->SetName( L"Color Render Target" );
#endif

		Renderer::Get()->GetCommandList()->OMSetRenderTargets( 1, &m_RTVHeap->GetCPUDescriptorHandleForHeapStart(), false,
											&m_DSVHeap->GetCPUDescriptorHandleForHeapStart() );
		Renderer::Get()->GetCommandList()->RSSetViewports( 1, &m_Viewport );
		Renderer::Get()->GetCommandList()->RSSetScissorRects( 1, &m_ScissorRect );
	}

	void SceneRenderer::SetRenderingEnabled( bool Enabled )
	{
		m_RenderingEnabled = Enabled;
	}

}