#pragma once

#include "ForwardTypes.h"
#include "RenderBuffer.h"
#include "Runtime/Renderer/ResourceView.h"

namespace Drn
{
	class DecalBuffer : public RenderBuffer
	{
	public:
		DecalBuffer();
		virtual ~DecalBuffer();

		virtual void Init() override;
		virtual void Resize( const IntPoint& Size ) override;

		void Clear( class D3D12CommandList* CommandList );
		void Bind( class D3D12CommandList* CommandList );

		TRefCountPtr<RenderTexture2D> m_BaseColorTarget;

		TRefCountPtr<RenderTexture2D> m_NormalTarget;

		// Metallic Roughness AO
		TRefCountPtr<RenderTexture2D> m_MasksTarget;

	private:
	};
}