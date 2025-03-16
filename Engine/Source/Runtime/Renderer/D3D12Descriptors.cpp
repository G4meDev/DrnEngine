#include "DrnPCH.h"
#include "D3D12Descriptors.h"
#include "D3D12Device.h"

namespace Drn
{
	D3D12DescriptorHeap::D3D12DescriptorHeap(D3D12Device* InDevice, uint32 InNumDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE InType, ED3D12DescriptorHeapFlags InFlags, bool bInIsGlobal)
		: Device(InDevice)
		, Offset(0)
		, NumDescriptors(InNumDescriptors)
		, DescriptorSize(InDevice->GetD3DDevice()->GetDescriptorHandleIncrementSize(InType))
		, Type(InType)
		, Flags(InFlags)
		, bIsGlobal(bInIsGlobal)
		, SubAllocateSourceHeap(nullptr)
	{
		FreeBlocks.reserve(NumDescriptors);

		for (uint32 BlockIndex = InNumDescriptors; BlockIndex > 0; BlockIndex--)
		{
			FreeBlocks.push_back(BlockIndex - 1);
		}

		D3D12_DESCRIPTOR_HEAP_DESC Desc{};
		Desc.Type = InType;
		Desc.NumDescriptors = InNumDescriptors;
		Desc.Flags = EnumHasAnyFlags(InFlags, ED3D12DescriptorHeapFlags::GpuVisible) ? D3D12_DESCRIPTOR_HEAP_FLAGS::D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

		ID3D12DescriptorHeap* Heap_ptr;
		Device->GetD3DDevice()->CreateDescriptorHeap(&Desc, IID_PPV_ARGS(&Heap_ptr));
		
		Heap = std::shared_ptr<ID3D12DescriptorHeap>(Heap_ptr);

		//TempHeap->SetName(DebugName);

		CpuBase = Heap->GetCPUDescriptorHandleForHeapStart();
		GpuBase = EnumHasAnyFlags(InFlags, ED3D12DescriptorHeapFlags::GpuVisible) ? Heap->GetGPUDescriptorHandleForHeapStart() : D3D12_GPU_DESCRIPTOR_HANDLE{};
	}

	D3D12DescriptorHeap::D3D12DescriptorHeap(D3D12DescriptorHeap* InSubAllocateSourceHeap)
 		: Device(InSubAllocateSourceHeap->GetDevice())
  		, Offset(InSubAllocateSourceHeap->Alloc())
 		, NumDescriptors(1)
 		, DescriptorSize(InSubAllocateSourceHeap->DescriptorSize)
 		, Type(InSubAllocateSourceHeap->Type)
 		, Flags(InSubAllocateSourceHeap->Flags)
 		, bIsGlobal(InSubAllocateSourceHeap->bIsGlobal)
 		, SubAllocateSourceHeap(InSubAllocateSourceHeap)
 	{
 		CpuBase = CD3DX12_CPU_DESCRIPTOR_HANDLE(InSubAllocateSourceHeap->CpuBase, Offset, InSubAllocateSourceHeap->DescriptorSize);
 		GpuBase = CD3DX12_GPU_DESCRIPTOR_HANDLE(InSubAllocateSourceHeap->GpuBase, Offset, InSubAllocateSourceHeap->DescriptorSize);
 	}

	D3D12DescriptorHeap::~D3D12DescriptorHeap()
	{
		if (SubAllocateSourceHeap == nullptr)
		{
			Heap->Release();
		}

		else
		{
			SubAllocateSourceHeap->Free(this);
		}

	}

	UINT D3D12DescriptorHeap::Alloc()
	{
		VERIFYD3D12RESULT(FreeBlocks.size() == 0);

		UINT Result = FreeBlocks.back();
		FreeBlocks.pop_back();
		
		return Result;
	}

	void D3D12DescriptorHeap::Free(D3D12DescriptorHeap* HeapToFree)
	{
		FreeBlocks.push_back(HeapToFree->Offset);
	}

	void D3D12DescriptorHeap::Free(D3D12_CPU_DESCRIPTOR_HANDLE CpuHandle)
	{
		int BlockIndex = (int)((CpuHandle.ptr - CpuBase.ptr) / DescriptorSize);
		FreeBlocks.push_back(BlockIndex);
	}

	void D3D12DescriptorHeap::Free(D3D12_GPU_DESCRIPTOR_HANDLE GpuHandle)
	{
		int BlockIndex = (int)((GpuHandle.ptr - GpuBase.ptr) / DescriptorSize);
		FreeBlocks.push_back(BlockIndex);
	}
}