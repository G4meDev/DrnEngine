#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class IndexBuffer
	{
	public:
		IndexBuffer();
		virtual ~IndexBuffer();

		static IndexBuffer* Create(ID3D12GraphicsCommandList2* CommandList, const void* Source, uint32 IndexCount,
			uint32 BufferSize, DXGI_FORMAT Format, const std::string& Name, bool CreateOnDefaultHeap = true);

		void Bind(ID3D12GraphicsCommandList2* CommandList);

		uint64 m_IndexCount;

		class Resource* m_IndexBuffer;
		D3D12_INDEX_BUFFER_VIEW m_IndexBufferView;
	};
}