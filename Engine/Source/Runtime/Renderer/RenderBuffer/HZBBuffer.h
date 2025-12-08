#pragma once

#include "ForwardTypes.h"
#include "RenderBuffer.h"

namespace Drn
{
	class HZBBuffer : public RenderBuffer
	{
	public:
		HZBBuffer();
		virtual ~HZBBuffer();

		virtual void Init() override;
		virtual void Resize( const IntPoint& Size ) override;

		void Clear( class D3D12CommandList* CommandList );
		void Bind( class D3D12CommandList* CommandList );

		void ReallocateViewHandles();

		TRefCountPtr<RenderTexture2D> M_HZBTarget;

		std::vector<TRefCountPtr<UnorderedAccessView>> m_UAVHandles;
		std::vector<TRefCountPtr<ShaderResourceView>> m_SRVHandles;

		IntPoint m_FirstMipSize;
		int32 m_MipCount;
	};
}