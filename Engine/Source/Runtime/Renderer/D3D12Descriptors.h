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
		//D3D12DescriptorHeap() = delete;
		~D3D12DescriptorHeap();

		D3D12DescriptorHeap(D3D12Device* InDevice, uint32 InNumDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE InType, ED3D12DescriptorHeapFlags InFlags, bool bInIsGlobal);
		D3D12DescriptorHeap(D3D12DescriptorHeap* InSubAllocateSourceHeap);

		inline D3D12Device* GetDevice() { return Device; }
	
		inline CD3DX12_CPU_DESCRIPTOR_HANDLE GetCpuHandle() { return CpuBase; };
		inline CD3DX12_GPU_DESCRIPTOR_HANDLE GetGpuHandle() { return GpuBase; };

		inline ID3D12DescriptorHeap* GetHeap() 
		{
			if (SubAllocateSourceHeap == nullptr)
			{
				return Heap.get();
			}

			return SubAllocateSourceHeap->Heap.get();
		}

		UINT Alloc();
		void Free(D3D12DescriptorHeap* HeapToFree);
		void Free(D3D12_CPU_DESCRIPTOR_HANDLE CpuHandle);
		void Free(D3D12_GPU_DESCRIPTOR_HANDLE GpuHandle);

	protected:


	private:

		D3D12Device* Device;
		std::shared_ptr<ID3D12DescriptorHeap> Heap;

		CD3DX12_CPU_DESCRIPTOR_HANDLE CpuBase;
		CD3DX12_GPU_DESCRIPTOR_HANDLE GpuBase;

		UINT Offset;
		UINT NumDescriptors;
		UINT DescriptorSize;
		D3D12_DESCRIPTOR_HEAP_TYPE Type;
		ED3D12DescriptorHeapFlags Flags;
		bool bIsGlobal;
		D3D12DescriptorHeap* SubAllocateSourceHeap;

		std::vector<UINT> FreeBlocks;
	};
}