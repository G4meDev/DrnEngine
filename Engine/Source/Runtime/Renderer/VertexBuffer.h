#pragma once

#include "ForwardTypes.h"
#include "Runtime/Engine/StaticMeshVertexData.h"
#include "Runtime/Renderer/RenderResource.h"

namespace Drn
{
	class StaticMeshVertexBuffer
	{
	public:
		StaticMeshVertexBuffer();
		virtual ~StaticMeshVertexBuffer();

		static StaticMeshVertexBuffer* Create(class D3D12CommandList* CommandList, StaticMeshVertexData& Source, const std::string& Name, bool CreateOnDefaultHeap = true);

		void Bind(class D3D12CommandList* CommandList);

		TRefCountPtr<class RenderVertexBuffer> m_PositionBuffer;
		TRefCountPtr<class RenderVertexBuffer> m_NormalBuffer;
		TRefCountPtr<class RenderVertexBuffer> m_TangentBuffer;
		TRefCountPtr<class RenderVertexBuffer> m_BitTangentBuffer;
		TRefCountPtr<class RenderVertexBuffer> m_ColorBuffer;

		TRefCountPtr<class RenderVertexBuffer> m_UV1Buffer;
		TRefCountPtr<class RenderVertexBuffer> m_UV2Buffer;
		TRefCountPtr<class RenderVertexBuffer> m_UV3Buffer;
		TRefCountPtr<class RenderVertexBuffer> m_UV4Buffer;

		static D3D12_VERTEX_BUFFER_VIEW VertexBufferViewNull;
	};
}