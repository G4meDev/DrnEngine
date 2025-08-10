#include "DrnPCH.h"
#include "IndexBuffer.h"

namespace Drn
{
	IndexBuffer::IndexBuffer()
		: m_IndexBuffer(nullptr)
		, m_IndexCount(0)
	{
	}

	IndexBuffer::~IndexBuffer()
	{
		if (m_IndexBuffer)
		{
			m_IndexBuffer->ReleaseBufferedResource();
		}
	}

	IndexBuffer* IndexBuffer::Create( ID3D12GraphicsCommandList2* CommandList, const void* Source, uint32 IndexCount,
			uint32 BufferSize, DXGI_FORMAT Format, const std::string& Name, bool CreateOnDefaultHeap)
	{
		IndexBuffer* Result = new IndexBuffer();
		Result->m_IndexCount = IndexCount;

		if (CreateOnDefaultHeap)
		{
			Resource* IntermediateIndexBuffer = Resource::Create( D3D12_HEAP_TYPE_UPLOAD, CD3DX12_RESOURCE_DESC::Buffer( BufferSize ), D3D12_RESOURCE_STATE_GENERIC_READ, false);
			Result->m_IndexBuffer = Resource::Create( D3D12_HEAP_TYPE_DEFAULT, CD3DX12_RESOURCE_DESC::Buffer( BufferSize ), D3D12_RESOURCE_STATE_COMMON, false);

#if D3D12_Debug_INFO
			IntermediateIndexBuffer->SetName( Name + "_IntermediateIndexBuffer" );
			Result->m_IndexBuffer->SetName( Name + "_IndexBuffer" );
#endif

			UINT8*        pIndexDataBegin;
			CD3DX12_RANGE readRange( 0, 0 );
			IntermediateIndexBuffer->GetD3D12Resource()->Map( 0, &readRange, reinterpret_cast<void**>( &pIndexDataBegin ) );
			memcpy( pIndexDataBegin, Source, BufferSize );
			IntermediateIndexBuffer->GetD3D12Resource()->Unmap( 0, nullptr );

			CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition( Result->m_IndexBuffer->GetD3D12Resource(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST );
			CommandList->ResourceBarrier( 1, &barrier );

			CommandList->CopyResource( Result->m_IndexBuffer->GetD3D12Resource(), IntermediateIndexBuffer->GetD3D12Resource() );

			barrier = CD3DX12_RESOURCE_BARRIER::Transition( Result->m_IndexBuffer->GetD3D12Resource(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER );
			CommandList->ResourceBarrier( 1, &barrier );

			Result->m_IndexBufferView.BufferLocation = Result->m_IndexBuffer->GetD3D12Resource()->GetGPUVirtualAddress();
			Result->m_IndexBufferView.Format      = Format;
			Result->m_IndexBufferView.SizeInBytes = BufferSize;

			IntermediateIndexBuffer->ReleaseBufferedResource();
		}
		else
		{
			
		}

		return Result;
	}

	void IndexBuffer::Bind( ID3D12GraphicsCommandList2* CommandList )
	{
		CommandList->IASetIndexBuffer(&m_IndexBufferView);
	}

}