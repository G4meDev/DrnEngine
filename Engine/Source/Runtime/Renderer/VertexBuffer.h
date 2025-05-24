#pragma once

#include "ForwardTypes.h"

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
}