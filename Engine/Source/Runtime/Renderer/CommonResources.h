#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class ScreenTriangle
	{
	public:

		ScreenTriangle( ID3D12GraphicsCommandList2* CommandList );
		~ScreenTriangle();

		Resource* m_VertexBuffer;
		Resource* m_IndexBuffer;

		D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;
		D3D12_INDEX_BUFFER_VIEW m_IndexBufferView;
	};

	class CommonResources
	{
	public:

		CommonResources( ID3D12GraphicsCommandList2* CommandList );
		~CommonResources();

		static void Init( ID3D12GraphicsCommandList2* CommandList );
		static void Shutdown();

		inline static CommonResources* Get() { return m_SingletonInstance; }

		ScreenTriangle* m_ScreenTriangle;

	private:

		static CommonResources* m_SingletonInstance;
	};
}