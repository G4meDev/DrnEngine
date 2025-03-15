#pragma once

namespace Drn
{
	class D3D12Device;

	enum class ED3D12DescriptorHeapFlags : uint8
	{
		None = 0,
		GpuVisible = 1 << 0,
		Poolable = 1 << 1,
	};

	class D3D12DescriptorHeap
	{
	public:
		D3D12DescriptorHeap() = delete;

		D3D12DescriptorHeap(D3D12Device* InDevice, std::shared_ptr<ID3D12DescriptorHeap> InHeap, uint32 InNumDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE InType, ED3D12DescriptorHeapFlags InFlags, bool bInIsGlobal);
		D3D12DescriptorHeap(D3D12DescriptorHeap* SubAllocateSourceHeap, uint32 InOffset, uint32 InNumDescriptors);

		inline D3D12Device* GetDevice() { return Device; }

		static D3D12DescriptorHeap* Create(D3D12Device* Device, const TCHAR* DebugName, D3D12_DESCRIPTOR_HEAP_TYPE HeapType, uint32 NumDescriptors, ED3D12DescriptorHeapFlags Flags, bool bGlobal = false);
	
		inline CD3DX12_CPU_DESCRIPTOR_HANDLE GetCpuHandle() { return CpuBase; };

		inline ID3D12DescriptorHeap* GetHeap() { return Heap.get(); }

	protected:


	private:
		

		D3D12Device* Device;
		std::shared_ptr<ID3D12DescriptorHeap> Heap;

		const CD3DX12_CPU_DESCRIPTOR_HANDLE CpuBase;
		const CD3DX12_GPU_DESCRIPTOR_HANDLE GpuBase;

		UINT Offset;
		UINT NumDescriptors;
		UINT DescriptorSize;
		D3D12_DESCRIPTOR_HEAP_TYPE Type;
		ED3D12DescriptorHeapFlags Flags;
		bool bIsGlobal;
		bool bIsSubAllocation;
	};
}