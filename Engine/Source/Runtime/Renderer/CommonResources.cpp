#include "DrnPCH.h"
#include "CommonResources.h"

namespace Drn
{
	CommonResources* CommonResources::m_SingletonInstance = nullptr;

	CommonResources::CommonResources( ID3D12GraphicsCommandList2* CommandList )
	{
		m_ScreenTriangle = new ScreenTriangle( CommandList );
	}

	CommonResources::~CommonResources()
	{
		if (m_ScreenTriangle)
		{
			delete m_ScreenTriangle;
			m_ScreenTriangle = nullptr;
		}
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
		1, 1, 0, 1, 1,
		-1, 1, 0, -1, 1,
		1, -1, 0, 1, -1
	};

	uint32 TriangleIndexData[] = 
	{
		0, 1, 2
	};


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

}