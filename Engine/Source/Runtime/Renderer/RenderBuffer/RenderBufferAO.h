#pragma once

#include "ForwardTypes.h"
#include "RenderBuffer.h"

namespace Drn
{
    class RenderBufferAO : public RenderBuffer
    {
    public:
        RenderBufferAO();
        virtual ~RenderBufferAO();

        virtual void Init() override;
        virtual void Resize( const IntPoint& Size ) override;

        virtual void Clear( ID3D12GraphicsCommandList2* CommandList ) override;
        virtual void Bind( ID3D12GraphicsCommandList2* CommandList ) override;

        Resource* m_AOTarget;

        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_AORtvHeap;

        D3D12_CPU_DESCRIPTOR_HANDLE m_AOHandle;

        D3D12_VIEWPORT m_Viewport;
        D3D12_RECT     m_ScissorRect;

    private:
    };
}