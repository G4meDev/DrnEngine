#include "DrnPCH.h"
#include "D3D12Descriptors.h"
#include "D3D12Device.h"

namespace Drn
{
	D3D12DescriptorHeap::D3D12DescriptorHeap(D3D12Device* InDevice, std::shared_ptr<ID3D12DescriptorHeap> InHeap, uint32 InNumDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE InType, ED3D12DescriptorHeapFlags InFlags, bool bInIsGlobal)
		: Device(InDevice)
		, Heap(std::forward<std::shared_ptr<ID3D12DescriptorHeap>>(InHeap))
		, CpuBase(Heap->GetCPUDescriptorHandleForHeapStart())
		, GpuBase(EnumHasAnyFlags(InFlags, ED3D12DescriptorHeapFlags::GpuVisible) ? Heap->GetGPUDescriptorHandleForHeapStart() : D3D12_GPU_DESCRIPTOR_HANDLE{})
		, Offset(0)
		, NumDescriptors(InNumDescriptors)
		, DescriptorSize(InDevice->GetD3DDevice()->GetDescriptorHandleIncrementSize(InType))
		, Type(InType)
		, Flags(InFlags)
		, bIsGlobal(bInIsGlobal)
		, bIsSubAllocation(false)
	{
		
	}

	D3D12DescriptorHeap::D3D12DescriptorHeap(D3D12DescriptorHeap* SubAllocateSourceHeap, uint32 InOffset, uint32 InNumDescriptors)
		: Device(SubAllocateSourceHeap->GetDevice())
		, Heap(SubAllocateSourceHeap->Heap)
		, CpuBase(SubAllocateSourceHeap->CpuBase, InOffset, SubAllocateSourceHeap->DescriptorSize)
		, GpuBase(SubAllocateSourceHeap->GpuBase, InOffset, SubAllocateSourceHeap->DescriptorSize)
		, Offset(InOffset)
		, NumDescriptors(InNumDescriptors)
		, DescriptorSize(SubAllocateSourceHeap->DescriptorSize)
		, Type(SubAllocateSourceHeap->Type)
		, Flags(SubAllocateSourceHeap->Flags)
		, bIsGlobal(SubAllocateSourceHeap->bIsGlobal)
		, bIsSubAllocation(true)
	{
		
	}

	D3D12DescriptorHeap* D3D12DescriptorHeap::Create(D3D12Device* Device, const TCHAR* DebugName, D3D12_DESCRIPTOR_HEAP_TYPE HeapType, uint32 NumDescriptors, ED3D12DescriptorHeapFlags Flags, bool bGlobal)
	{
		D3D12_DESCRIPTOR_HEAP_DESC Desc{};
		Desc.Type = HeapType;
		Desc.NumDescriptors = NumDescriptors;
		Desc.Flags = EnumHasAnyFlags(Flags, ED3D12DescriptorHeapFlags::GpuVisible) ? D3D12_DESCRIPTOR_HEAP_FLAGS::D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

		ID3D12DescriptorHeap* Heap_ptr;
		Device->GetD3DDevice()->CreateDescriptorHeap(&Desc, IID_PPV_ARGS(&Heap_ptr));
		
		std::shared_ptr<ID3D12DescriptorHeap> TempHeap(Heap_ptr);
		
		TempHeap->SetName(DebugName);

		return new D3D12DescriptorHeap(Device, std::move(TempHeap), NumDescriptors, HeapType, Flags, bGlobal);
	}

}