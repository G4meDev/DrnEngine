#pragma once

#include "ForwardTypes.h"
#include "Runtime/Engine/StaticMeshVertexData.h"

namespace Drn
{

	class VertexBuffer
	{
	public:
		VertexBuffer();
		virtual ~VertexBuffer();

		static VertexBuffer* Create(ID3D12GraphicsCommandList2* CommandList, const void* Source, uint32 VertexCount, uint32 Stride, const std::string& Name, bool CreateOnDefaultHeap = true);

		void Bind(ID3D12GraphicsCommandList2* CommandList);

		class Resource* m_VertexBuffer;
		D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;
	};

	class StaticMeshVertexBuffer
	{
	public:
		StaticMeshVertexBuffer();
		virtual ~StaticMeshVertexBuffer();

		static StaticMeshVertexBuffer* Create(ID3D12GraphicsCommandList2* CommandList, StaticMeshVertexData& Source, const std::string& Name, bool CreateOnDefaultHeap = true);

		void Bind(ID3D12GraphicsCommandList2* CommandList);

		class VertexBuffer* m_PositionBuffer;
		class VertexBuffer* m_NormalBuffer;
		class VertexBuffer* m_TangentBuffer;
		class VertexBuffer* m_BitTangentBuffer;
		class VertexBuffer* m_ColorBuffer;

		class VertexBuffer* m_UV1Buffer;
		class VertexBuffer* m_UV2Buffer;
		class VertexBuffer* m_UV3Buffer;
		class VertexBuffer* m_UV4Buffer;

		static D3D12_VERTEX_BUFFER_VIEW VertexBufferViewNull;
	};
}