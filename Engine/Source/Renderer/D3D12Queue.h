#pragma once

namespace Drn
{
	class D3D12Device;

	enum class D3D12QueueType
	{
		Direct = 0,
		Copy,
		Compute,

		Count,
	};

	inline D3D12_COMMAND_LIST_TYPE GetD3DCommandListType(D3D12QueueType QueueType)
	{
		switch (QueueType)
		{
		case D3D12QueueType::Direct: return D3D12_COMMAND_LIST_TYPE_DIRECT;
		case D3D12QueueType::Copy:   return D3D12_COMMAND_LIST_TYPE_COPY;
		case D3D12QueueType::Compute:  return D3D12_COMMAND_LIST_TYPE_COMPUTE;
		default:
			return D3D12_COMMAND_LIST_TYPE_DIRECT;
		}
	}

	class D3D12Queue
	{
	public:

		D3D12Queue(D3D12Device* Device, D3D12QueueType QueueType);

		~D3D12Queue();

		D3D12Device* const Device;
		D3D12QueueType const QueueType;

		Microsoft::WRL::ComPtr<ID3D12CommandQueue> CommandQueue;

		//ID3D12Fence Fence;

	protected:

	private:
	};

}