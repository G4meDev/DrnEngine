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
		Renderer::Get()->TempSRVAllocator.Free(m_EditorColorCpuHandle, m_EditorColorGpuHandle);
	}

	void SceneRenderer::Init(ID3D12GraphicsCommandList2* CommandList)
	{
		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
		dsvHeapDesc.NumDescriptors = 2;
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		Device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_DSVHeap));

		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = 3;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		Device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_RTVHeap));

#if D3D12_Debug_INFO
		m_DSVHeap->SetName( StringHelper::s2ws( m_Name + "_DsvHeap").c_str() );
		m_RTVHeap->SetName( StringHelper::s2ws( m_Name + "_RtvHeap").c_str() );
#endif

		Renderer::Get()->TempSRVAllocator.Alloc(&m_EditorColorCpuHandle, &m_EditorColorGpuHandle);

		ResizeView(IntPoint(1920, 1080));
	}

	void SceneRenderer::BeginRender(ID3D12GraphicsCommandList2* CommandList)
	{
		SCOPE_STAT( BeginRender );

		FLOAT clearColor[] = { 0.4f, 0.6f, 0.9f, 1.0f };
		FLOAT GuidColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };

		uint32 RTVDescriporSize = Renderer::Get()->GetD3D12Device()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		CD3DX12_CPU_DESCRIPTOR_HANDLE GuidHandle(m_RTVHeap->GetCPUDescriptorHandleForHeapStart(), RTVDescriporSize);

		D3D12_CPU_DESCRIPTOR_HANDLE const RTVHandles[2] = 
		{
			m_RTVHeap->GetCPUDescriptorHandleForHeapStart(),
			CD3DX12_CPU_DESCRIPTOR_HANDLE(m_RTVHeap->GetCPUDescriptorHandleForHeapStart(), 1, RTVDescriporSize)
		};

		CommandList->ClearDepthStencilView(m_DSVHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1, 0, 0, nullptr);
		CommandList->ClearRenderTargetView(m_RTVHeap->GetCPUDescriptorHandleForHeapStart(), clearColor, 0, nullptr);
		CommandList->ClearRenderTargetView(GuidHandle, GuidColor, 0, nullptr);

		CommandList->OMSetRenderTargets(2, RTVHandles, false, &m_DSVHeap->GetCPUDescriptorHandleForHeapStart());
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

	void SceneRenderer::RenderEditorPrimitives( ID3D12GraphicsCommandList2* CommandList )
	{
		SCOPE_STAT( RenderEditorPrimitives );

		FLOAT clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };

		uint32 RTVDescriporSize = Renderer::Get()->GetD3D12Device()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		uint32 DSVDescriporSize = Renderer::Get()->GetD3D12Device()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

		CD3DX12_CPU_DESCRIPTOR_HANDLE EditorColorHandle(m_RTVHeap->GetCPUDescriptorHandleForHeapStart(), 2, RTVDescriporSize);
		CD3DX12_CPU_DESCRIPTOR_HANDLE EditorDepthHandle(m_DSVHeap->GetCPUDescriptorHandleForHeapStart(), 1, DSVDescriporSize);

		D3D12_CPU_DESCRIPTOR_HANDLE const RTVHandles[1] = 
		{
			EditorColorHandle
		};

		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			m_DepthTarget.Get(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_COPY_SOURCE );
		CommandList->ResourceBarrier(1, &barrier);

		barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			m_EditorDepthTarget.Get(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_COPY_DEST );
		CommandList->ResourceBarrier(1, &barrier);

		CommandList->CopyResource(m_EditorDepthTarget.Get(), m_DepthTarget.Get());

		barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			m_DepthTarget.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE );
		CommandList->ResourceBarrier(1, &barrier);

		barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			m_EditorDepthTarget.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_DEPTH_WRITE );
		CommandList->ResourceBarrier(1, &barrier);

		CommandList->ClearRenderTargetView(EditorColorHandle, clearColor, 0, nullptr);
		CommandList->OMSetRenderTargets(1, RTVHandles, false, &EditorDepthHandle);

		for (PrimitiveSceneProxy* Proxy : m_Scene->m_EditorPrimitiveProxies)
		{
			Proxy->RenderMainPass(CommandList, this);
		}

// ------------------------------------------------------------------------------------------

		CommandList->OMSetRenderTargets(1, &m_RTVHeap->GetCPUDescriptorHandleForHeapStart(), true, NULL);

		barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			m_EditorColorTarget.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE );
		CommandList->ResourceBarrier(1, &barrier);

		CommandList->SetGraphicsRootSignature( CommonResources::Get()->m_ResolveAlphaBlendedPSO->m_RootSignature );
		CommandList->SetPipelineState( CommonResources::Get()->m_ResolveAlphaBlendedPSO->m_PSO );

		XMMATRIX modelMatrix = Matrix( m_CameraActor->GetActorTransform() ).Get();
		XMMATRIX viewMatrix;
		XMMATRIX projectionMatrix;
		float aspectRatio = (float) m_RenderSize.X / m_RenderSize.Y;
		m_CameraActor->GetCameraComponent()->CalculateMatrices(viewMatrix, projectionMatrix, aspectRatio);
		XMMATRIX LocalToView = XMMatrixMultiply( modelMatrix, viewMatrix );

		CommandList->SetGraphicsRoot32BitConstants(0, 16, &LocalToView, 0);
		CommandList->SetGraphicsRootDescriptorTable(1, m_EditorColorGpuHandle);

		CommandList->IASetVertexBuffers( 0, 1, &CommonResources::Get()->m_ScreenTriangle->m_VertexBufferView );
		CommandList->IASetIndexBuffer( &CommonResources::Get()->m_ScreenTriangle->m_IndexBufferView );
		CommandList->DrawIndexedInstanced( 3, 1, 0, 0, 0 );

		barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			m_EditorColorTarget.Get(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET );
		CommandList->ResourceBarrier(1, &barrier);
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
		RenderEditorPrimitives(CommandList);
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

// ----------------------------------------------------------------------------------------------------------------------------

		D3D12_CLEAR_VALUE GuidClearValue;
		GuidClearValue.Format   = GBUFFER_GUID_FORMAT;
		GuidClearValue.Color[0] = 0.0f;
		GuidClearValue.Color[1] = 0.0f;
		GuidClearValue.Color[2] = 0.0f;
		GuidClearValue.Color[3] = 0.0f;

		Device->CreateCommittedResource( &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Tex2D(GBUFFER_GUID_FORMAT, m_RenderSize.X, m_RenderSize.Y,
			1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET), D3D12_RESOURCE_STATE_RENDER_TARGET,
			&GuidClearValue, IID_PPV_ARGS(m_GuidTarget.ReleaseAndGetAddressOf()) );

		rtv.Format = GBUFFER_GUID_FORMAT;
		rtv.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		rtv.Texture2D.MipSlice = 0;

		uint32 RTVDescriporSize = Renderer::Get()->GetD3D12Device()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		uint32 DSVDescriporSize = Renderer::Get()->GetD3D12Device()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		Device->CreateRenderTargetView( m_GuidTarget.Get(), &rtv, CD3DX12_CPU_DESCRIPTOR_HANDLE(m_RTVHeap->GetCPUDescriptorHandleForHeapStart(), 1, RTVDescriporSize) );

#if D3D12_Debug_INFO
		m_GuidTarget->SetName( L"Guid Target" );
#endif

// ----------------------------------------------------------------------------------------------------------------------------

		DXGI_FORMAT EditorColorFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

		D3D12_CLEAR_VALUE EditorcolorClearValue;
		EditorcolorClearValue.Format   = EditorColorFormat;
		EditorcolorClearValue.Color[0] = 0.0f;
		EditorcolorClearValue.Color[1] = 0.0f;
		EditorcolorClearValue.Color[2] = 0.0f;
		EditorcolorClearValue.Color[3] = 0.0f;

		Device->CreateCommittedResource( &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Tex2D(EditorColorFormat, m_RenderSize.X, m_RenderSize.Y,
			1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET), D3D12_RESOURCE_STATE_RENDER_TARGET,
			&EditorcolorClearValue, IID_PPV_ARGS(m_EditorColorTarget.ReleaseAndGetAddressOf()) );

		D3D12_RENDER_TARGET_VIEW_DESC rtv2 = {};
		rtv2.Format = EditorColorFormat;
		rtv2.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		rtv2.Texture2D.MipSlice = 0;

		Device->CreateRenderTargetView(m_EditorColorTarget.Get(), &rtv2, CD3DX12_CPU_DESCRIPTOR_HANDLE(m_RTVHeap->GetCPUDescriptorHandleForHeapStart(), 2, RTVDescriporSize) );

#if D3D12_Debug_INFO
		m_EditorColorTarget->SetName( L"Editor Color Render Target" );
#endif

// ----------------------------------------------------------------------------------------------------------------------------

		D3D12_CLEAR_VALUE EditorDepthClearValue = {};
		EditorDepthClearValue.Format = DEPTH_FORMAT;
		EditorDepthClearValue.DepthStencil = { 1.0f, 0 };

		Device->CreateCommittedResource( &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Tex2D(DEPTH_FORMAT, m_RenderSize.X, m_RenderSize.Y,
			1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL), D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&EditorDepthClearValue, IID_PPV_ARGS(m_EditorDepthTarget.ReleaseAndGetAddressOf()) );

		D3D12_DEPTH_STENCIL_VIEW_DESC dsv2 = {};
		dsv2.Format = DEPTH_FORMAT;
		dsv2.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsv2.Texture2D.MipSlice = 0;
		dsv2.Flags = D3D12_DSV_FLAG_NONE;

		Device->CreateDepthStencilView(m_EditorDepthTarget.Get(), &dsv2, CD3DX12_CPU_DESCRIPTOR_HANDLE(m_DSVHeap->GetCPUDescriptorHandleForHeapStart(), 1, DSVDescriporSize) );

#if D3D12_Debug_INFO
		m_EditorDepthTarget->SetName( L"Editor Depth Render Target" );
#endif

// ----------------------------------------------------------------------------------------------------------------------------

		D3D12_SHADER_RESOURCE_VIEW_DESC SrvDesc = {};
		SrvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		SrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		SrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		SrvDesc.Texture2D.MipLevels = 1;
		SrvDesc.Texture2D.MostDetailedMip = 0;

		Device->CreateShaderResourceView( m_EditorColorTarget.Get(), &SrvDesc, m_EditorColorCpuHandle );

// ----------------------------------------------------------------------------------------------------------------------------
		D3D12_CPU_DESCRIPTOR_HANDLE const RTVHandles[2] = 
		{
			m_RTVHeap->GetCPUDescriptorHandleForHeapStart(),
			CD3DX12_CPU_DESCRIPTOR_HANDLE(m_RTVHeap->GetCPUDescriptorHandleForHeapStart(), 1, RTVDescriporSize)
		};

		Renderer::Get()->GetCommandList()->OMSetRenderTargets( 2, RTVHandles, false, &m_DSVHeap->GetCPUDescriptorHandleForHeapStart() );
		Renderer::Get()->GetCommandList()->RSSetViewports( 1, &m_Viewport );
		Renderer::Get()->GetCommandList()->RSSetScissorRects( 1, &m_ScissorRect );
	}

	void SceneRenderer::SetRenderingEnabled( bool Enabled )
	{
		m_RenderingEnabled = Enabled;
	}

	Guid SceneRenderer::GetGuidAtScreenPosition( const IntPoint& ScreenPosition )
	{
		Guid Result;

		if (m_GuidTarget)
		{
			ID3D12Device* Device = Renderer::Get()->GetD3D12Device();
			ID3D12GraphicsCommandList2* CommandList = Renderer::Get()->GetCommandList();

			Microsoft::WRL::ComPtr<ID3D12Resource> ReadBackResource;
			Device->CreateCommittedResource( &CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_READBACK ), D3D12_HEAP_FLAG_NONE,
				&CD3DX12_RESOURCE_DESC::Buffer(16), D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(ReadBackResource.GetAddressOf()));

			CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition( 
				m_GuidTarget.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE );
			CommandList->ResourceBarrier(1, &barrier);

			const IntPoint ClampedPos = IntPoint( 
				std::clamp<int32>(ScreenPosition.X, 0, m_RenderSize.X - 1),
				std::clamp<int32>(ScreenPosition.Y, 0, m_RenderSize.Y - 1));

			CD3DX12_BOX CopyBox(ClampedPos.X, ClampedPos.Y, ClampedPos.X + 1, ClampedPos.Y + 1);

			D3D12_PLACED_SUBRESOURCE_FOOTPRINT Footprint = {};
			Footprint.Footprint.Format = GBUFFER_GUID_FORMAT;
			Footprint.Footprint.Width = 1;
			Footprint.Footprint.Height = 1;
			Footprint.Footprint.Depth = 1;
			Footprint.Footprint.RowPitch = 256;
			Footprint.Offset = 0;

			CD3DX12_TEXTURE_COPY_LOCATION SourceLoc(m_GuidTarget.Get(), 0);
			CD3DX12_TEXTURE_COPY_LOCATION DestLoc(ReadBackResource.Get(), Footprint);

			CommandList->CopyTextureRegion(&DestLoc, 0, 0, 0, &SourceLoc, &CopyBox);

			barrier = CD3DX12_RESOURCE_BARRIER::Transition( 
				m_GuidTarget.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET );
			CommandList->ResourceBarrier(1, &barrier);

			Renderer::Get()->Flush();

			UINT8* MemoryStart;

			D3D12_RANGE ReadRange = {};
			ReadRange.Begin = 0;
			ReadRange.End = 16;
			ReadBackResource->Map(0, &ReadRange, reinterpret_cast<void**>(&MemoryStart));

			memcpy(&Result, MemoryStart, 16);
		}

		return Result;
	}

}