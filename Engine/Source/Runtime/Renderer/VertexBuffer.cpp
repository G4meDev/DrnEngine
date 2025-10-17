#include "DrnPCH.h"
#include "VertexBuffer.h"
#include "Runtime/Renderer/Resource.h"

namespace Drn
{
	VertexBuffer::VertexBuffer()
		: m_VertexBuffer(nullptr)
	{
		m_VertexBufferView.BufferLocation = 0;
		m_VertexBufferView.StrideInBytes  = 0;
		m_VertexBufferView.SizeInBytes    = 0;
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
				CD3DX12_RESOURCE_DESC::Buffer( VertexBufferSize ), D3D12_RESOURCE_STATE_COMMON, false);

			Resource* IntermediateVertexBuffer = Resource::Create(D3D12_HEAP_TYPE_UPLOAD, 
				CD3DX12_RESOURCE_DESC::Buffer( VertexBufferSize ), D3D12_RESOURCE_STATE_GENERIC_READ, false);

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

	StaticMeshVertexBuffer::StaticMeshVertexBuffer()
		: m_PositionBuffer(nullptr)
		, m_NormalBuffer(nullptr)
		, m_TangentBuffer(nullptr)
		, m_BitTangentBuffer(nullptr)
		, m_ColorBuffer(nullptr)
		, m_UV1Buffer(nullptr)
		, m_UV2Buffer(nullptr)
		, m_UV3Buffer(nullptr)
		, m_UV4Buffer(nullptr)
	{}

	StaticMeshVertexBuffer::~StaticMeshVertexBuffer()
	{
		auto ReleaseBuffer = [](VertexBuffer*& VBuffer) { if (VBuffer) {delete VBuffer;} };

		ReleaseBuffer(m_PositionBuffer);
		ReleaseBuffer(m_NormalBuffer);
		ReleaseBuffer(m_TangentBuffer);
		ReleaseBuffer(m_BitTangentBuffer);
		ReleaseBuffer(m_ColorBuffer);
		ReleaseBuffer(m_UV1Buffer);
		ReleaseBuffer(m_UV2Buffer);
		ReleaseBuffer(m_UV3Buffer);
		ReleaseBuffer(m_UV4Buffer);
	}

	StaticMeshVertexBuffer* StaticMeshVertexBuffer::Create( ID3D12GraphicsCommandList2* CommandList, StaticMeshVertexData& Source, const std::string& Name, bool CreateOnDefaultHeap )
	{
		StaticMeshVertexBuffer* Result = new StaticMeshVertexBuffer;

		if (Source.GetVertexCount() == 0)
		{
			return Result;
		}

		Result->m_PositionBuffer = VertexBuffer::Create(CommandList, Source.GetPositions().data(), Source.GetVertexCount(), sizeof(Vector), Name + "_Position", CreateOnDefaultHeap);

		if (Source.HasNormals())
			Result->m_NormalBuffer = VertexBuffer::Create(CommandList, Source.GetNormals().data(), Source.GetVertexCount(), sizeof(uint32), Name + "_Normal", CreateOnDefaultHeap);

		if (Source.HasTangents())
			Result->m_TangentBuffer = VertexBuffer::Create(CommandList, Source.GetTangents().data(), Source.GetVertexCount(), sizeof(uint32), Name + "_Tangent", CreateOnDefaultHeap);

		if (Source.HasBitTangents())
			Result->m_BitTangentBuffer = VertexBuffer::Create(CommandList, Source.GetBitTangents().data(), Source.GetVertexCount(), sizeof(uint32), Name + "_BitTangent", CreateOnDefaultHeap);

		if (Source.HasColors())
			Result->m_ColorBuffer = VertexBuffer::Create(CommandList, Source.GetColor().data(), Source.GetVertexCount(), sizeof(Color), Name + "_Color", CreateOnDefaultHeap);

		if (Source.HasUV1())
			Result->m_UV1Buffer = VertexBuffer::Create(CommandList, Source.GetUV1().data(), Source.GetVertexCount(), sizeof(Vector2Half), Name + "_UV1", CreateOnDefaultHeap);

		if (Source.HasUV2())
			Result->m_UV2Buffer = VertexBuffer::Create(CommandList, Source.GetUV2().data(), Source.GetVertexCount(), sizeof(Vector2Half), Name + "_UV2", CreateOnDefaultHeap);

		if (Source.HasUV3())
			Result->m_UV3Buffer = VertexBuffer::Create(CommandList, Source.GetUV3().data(), Source.GetVertexCount(), sizeof(Vector2Half), Name + "_UV3", CreateOnDefaultHeap);

		if (Source.HasUV4())
			Result->m_UV4Buffer = VertexBuffer::Create(CommandList, Source.GetUV4().data(), Source.GetVertexCount(), sizeof(Vector2Half), Name + "_UV4", CreateOnDefaultHeap);

		return Result;
	}

	void StaticMeshVertexBuffer::Bind( ID3D12GraphicsCommandList2* CommandList )
	{
		D3D12_VERTEX_BUFFER_VIEW Views[9] = 
		{
			m_PositionBuffer ? m_PositionBuffer->m_VertexBufferView : VertexBufferViewNull,
			m_ColorBuffer ? m_ColorBuffer->m_VertexBufferView : VertexBufferViewNull,
			m_NormalBuffer ? m_NormalBuffer->m_VertexBufferView : VertexBufferViewNull,
			m_TangentBuffer ? m_TangentBuffer->m_VertexBufferView : VertexBufferViewNull,
			m_BitTangentBuffer ? m_BitTangentBuffer->m_VertexBufferView : VertexBufferViewNull,
			m_UV1Buffer ? m_UV1Buffer->m_VertexBufferView : VertexBufferViewNull,
			m_UV2Buffer ? m_UV2Buffer->m_VertexBufferView : VertexBufferViewNull,
			m_UV3Buffer ? m_UV3Buffer->m_VertexBufferView : VertexBufferViewNull,
			m_UV4Buffer ? m_UV4Buffer->m_VertexBufferView : VertexBufferViewNull,
		};

		CommandList->IASetVertexBuffers(0, 9, Views);
	}

	D3D12_VERTEX_BUFFER_VIEW StaticMeshVertexBuffer::VertexBufferViewNull;
}