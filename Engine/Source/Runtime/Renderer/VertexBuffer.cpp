#include "DrnPCH.h"
#include "VertexBuffer.h"
#include "Runtime/Renderer/Resource.h"

namespace Drn
{
	VertexBuffer::VertexBuffer()
		: m_VertexBuffer(nullptr)
	{
	}

	VertexBuffer::~VertexBuffer()
	{
		if (m_VertexBuffer) { m_VertexBuffer->ReleaseBufferedResource(); }
	}

	VertexBuffer* VertexBuffer::Create(ID3D12GraphicsCommandList2* CommandList, const void* Source, uint32 VertexCount, uint32 Stride, const std::string& Name, bool CreateOnDefaultHeap)
	{
		VertexBuffer* Result = new VertexBuffer();

		if (CreateOnDefaultHeap)
		{
			const uint32 VertexBufferSize = VertexCount * Stride;

			Result->m_VertexBuffer = Resource::Create(D3D12_HEAP_TYPE_DEFAULT, 
				CD3DX12_RESOURCE_DESC::Buffer( VertexBufferSize ), D3D12_RESOURCE_STATE_COMMON);

			Resource* IntermediateVertexBuffer = Resource::Create(D3D12_HEAP_TYPE_UPLOAD, 
				CD3DX12_RESOURCE_DESC::Buffer( VertexBufferSize ), D3D12_RESOURCE_STATE_GENERIC_READ);

			UINT8* pVertexDataBegin;
			CD3DX12_RANGE readRange( 0, 0 );
			IntermediateVertexBuffer->GetD3D12Resource()->Map( 0, &readRange, reinterpret_cast<void**>( &pVertexDataBegin ) );
			memcpy( pVertexDataBegin, Source, VertexBufferSize );
			IntermediateVertexBuffer->GetD3D12Resource()->Unmap( 0, nullptr );

			CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
				Result->m_VertexBuffer->GetD3D12Resource(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST );
			CommandList->ResourceBarrier(1, &barrier);

			CommandList->CopyResource(Result->m_VertexBuffer->GetD3D12Resource(), IntermediateVertexBuffer->GetD3D12Resource());

			barrier = CD3DX12_RESOURCE_BARRIER::Transition(
				Result->m_VertexBuffer->GetD3D12Resource(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER );
			CommandList->ResourceBarrier(1, &barrier);

			Result->m_VertexBufferView.BufferLocation = Result->m_VertexBuffer->GetD3D12Resource()->GetGPUVirtualAddress();
			Result->m_VertexBufferView.StrideInBytes  = Stride;
			Result->m_VertexBufferView.SizeInBytes    = VertexBufferSize;

#if D3D12_Debug_INFO
			IntermediateVertexBuffer->SetName( Name + "_IntermediateVertexBuffer" );
			Result->m_VertexBuffer->SetName( Name + "_VertexBuffer" );
#endif

			IntermediateVertexBuffer->ReleaseBufferedResource();
		}
		else
		{

		}

		return Result;
	}

	void VertexBuffer::Bind( ID3D12GraphicsCommandList2* CommandList )
	{
		CommandList->IASetVertexBuffers(0, 1, &m_VertexBufferView);
	}

}