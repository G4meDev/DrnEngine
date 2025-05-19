#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class RenderBuffer
	{
	protected:
		RenderBuffer()
			: m_Size(IntPoint(1))
		{
		}

		virtual ~RenderBuffer()
		{
		}

	public:

		virtual void Init();
		virtual void Resize( const IntPoint& Size );

		virtual void Bind( ID3D12GraphicsCommandList2* CommandList );
		virtual void Clear( ID3D12GraphicsCommandList2* CommandList );

	protected:

		IntPoint m_Size;

	private:

	};
}