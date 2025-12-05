#pragma once

#include "ForwardTypes.h"
#include "Runtime/Renderer/BufferedResource.h"
#include "Runtime/Renderer/RenderCommon.h"

namespace Drn
{
	class D3D12CommandList : public BufferedResource, public DeviceChild
	{
	public:
		D3D12CommandList(class Device* Device, D3D12_COMMAND_LIST_TYPE Type, uint8 NumAllocators, const std::string& Name);
		virtual ~D3D12CommandList();

		inline ID3D12GraphicsCommandList2* GetD3D12CommandList() { return m_CommandList.Get(); }

		void Close();
		void FlipAndReset();
		void SetAllocatorAndReset(uint8 AllocatorIndex);

	protected:

	private:

		std::vector<Microsoft::WRL::ComPtr<ID3D12CommandAllocator>> m_CommandAllocators;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> m_CommandList;

		D3D12_COMMAND_LIST_TYPE m_Type;
		uint8 m_NumAllocators;

		uint8 m_CurrentAllocatorIndex;
	};
}