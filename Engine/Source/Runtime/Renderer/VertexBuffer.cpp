#include "DrnPCH.h"
#include "VertexBuffer.h"
#include "Runtime/Renderer/Resource.h"
#include "Runtime/Renderer/RenderBuffer.h"

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
	{}

	StaticMeshVertexBuffer* StaticMeshVertexBuffer::Create( D3D12CommandList* CommandList, StaticMeshVertexData& Source, const std::string& Name, bool CreateOnDefaultHeap )
	{
		StaticMeshVertexBuffer* Result = new StaticMeshVertexBuffer;

		if (Source.GetVertexCount() == 0)
		{
			return Result;
		}

		auto CreateVertexBufferConditional = [](bool bCreate, D3D12CommandList* CmdList, TRefCountPtr<class RenderVertexBuffer>& Buffer, void* Data, uint32 Size, const std::string& Name)
		{
			if (bCreate)
			{
				uint32 VertexBufferFlags = (uint32)EBufferUsageFlags::VertexBuffer | (uint32)EBufferUsageFlags::Static;
				RenderResourceCreateInfo VertexBufferCreateInfo(nullptr, Data, ClearValueBinding::Black, Name);
				Buffer = RenderVertexBuffer::Create(CmdList->GetParentDevice(), CmdList, Size, VertexBufferFlags, D3D12_RESOURCE_STATE_COMMON, false, VertexBufferCreateInfo);
			}
			else
			{
				Buffer = nullptr;
			}
		};

		CreateVertexBufferConditional(true, CommandList, Result->m_PositionBuffer, (void*)Source.GetPositions().data(), Source.GetVertexCount() * sizeof(Vector), "VB_Position");
		CreateVertexBufferConditional(Source.HasNormals(), CommandList, Result->m_NormalBuffer, (void*)Source.GetNormals().data(), Source.GetVertexCount() * sizeof(uint32), "VB_Normal");
		CreateVertexBufferConditional(Source.HasTangents(), CommandList, Result->m_TangentBuffer, (void*)Source.GetTangents().data(), Source.GetVertexCount() * sizeof(uint32), "VB_Tangent");
		CreateVertexBufferConditional(Source.HasBitTangents(), CommandList, Result->m_BitTangentBuffer, (void*)Source.GetBitTangents().data(), Source.GetVertexCount() * sizeof(uint32), "VB_BitTangent");
		CreateVertexBufferConditional(Source.HasColors(), CommandList, Result->m_ColorBuffer, (void*)Source.GetColor().data(), Source.GetVertexCount() * sizeof(Color), "VB_Color");
		CreateVertexBufferConditional(Source.HasUV1(), CommandList, Result->m_UV1Buffer, (void*)Source.GetUV1().data(), Source.GetVertexCount() * sizeof(Vector2Half), "VB_UV1");
		CreateVertexBufferConditional(Source.HasUV2(), CommandList, Result->m_UV2Buffer, (void*)Source.GetUV2().data(), Source.GetVertexCount() * sizeof(Vector2Half), "VB_UV2");
		CreateVertexBufferConditional(Source.HasUV3(), CommandList, Result->m_UV3Buffer, (void*)Source.GetUV3().data(), Source.GetVertexCount() * sizeof(Vector2Half), "VB_UV3");
		CreateVertexBufferConditional(Source.HasUV4(), CommandList, Result->m_UV4Buffer, (void*)Source.GetUV4().data(), Source.GetVertexCount() * sizeof(Vector2Half), "VB_UV4");

		return Result;
	}

	void StaticMeshVertexBuffer::Bind( D3D12CommandList* CommandList )
	{
		uint16 const Strides[] = { sizeof(Vector), sizeof(Color), sizeof(uint32), sizeof(uint32),  sizeof(uint32), sizeof(Vector2Half), sizeof(Vector2Half), sizeof(Vector2Half), sizeof(Vector2Half)};
		CommandList->SetStreamStrides(Strides);
		CommandList->SetStreamSource(0, m_PositionBuffer, 0);
		CommandList->SetStreamSource(1, m_ColorBuffer, 0);
		CommandList->SetStreamSource(2, m_NormalBuffer, 0);
		CommandList->SetStreamSource(3, m_TangentBuffer, 0);
		CommandList->SetStreamSource(4, m_BitTangentBuffer, 0);
		CommandList->SetStreamSource(5, m_UV1Buffer, 0);
		CommandList->SetStreamSource(6, m_UV2Buffer, 0);
		CommandList->SetStreamSource(7, m_UV3Buffer, 0);
		CommandList->SetStreamSource(8, m_UV4Buffer, 0);
	}

	D3D12_VERTEX_BUFFER_VIEW StaticMeshVertexBuffer::VertexBufferViewNull;
}