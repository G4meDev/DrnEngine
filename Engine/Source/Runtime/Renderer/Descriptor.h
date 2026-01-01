#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class Device;

	//template<bool OfflineHeap>
	class DescriptorManager : public DeviceChild
	{
	public:
		typedef D3D12_CPU_DESCRIPTOR_HANDLE HeapOffset;
		typedef decltype(HeapOffset::ptr) HeapOffsetRaw;
		typedef uint32 HeapIndex;
	
	private:
		struct SFreeRange { HeapOffsetRaw Start; HeapOffsetRaw End; };
		struct SHeapEntry
		{
			TRefCountPtr<ID3D12DescriptorHeap> m_Heap;
			std::list<SFreeRange> m_FreeList;

			SHeapEntry() { }
		};
		typedef std::vector<SHeapEntry> THeapMap;

		//template<bool OfflineHeap>
		static D3D12_DESCRIPTOR_HEAP_DESC CreateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE Type, uint32 NumDescriptorsPerHeap)
		{
			D3D12_DESCRIPTOR_HEAP_DESC Desc = {};
			Desc.Type = Type;
			Desc.NumDescriptors = NumDescriptorsPerHeap;
			//if constexpr (OfflineHeap)
			//{
				Desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			//}
			//else
			//{
			//	Desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			//}
			//Desc.NodeMask = 0;

			return Desc;
		}
	
	public:
		DescriptorManager(Device* Parent, D3D12_DESCRIPTOR_HEAP_TYPE Type, uint32 NumDescriptorsPerHeap)
			: DeviceChild(Parent)
			, m_Desc(CreateDescriptor(Type, NumDescriptorsPerHeap))
			, m_DescriptorSize(0)
		{}

		virtual ~DescriptorManager();

		void Init();

		HeapOffset AllocateHeapSlot( HeapIndex& outIndex );
		void FreeHeapSlot( HeapOffset Offset, HeapIndex index );

		int32 GetNumAllocatedHandles();

	private:
		void AllocateHeap();

		const D3D12_DESCRIPTOR_HEAP_DESC m_Desc;
		uint32 m_DescriptorSize;

		THeapMap m_Heaps;
		std::list<HeapIndex> m_FreeHeaps;
		CriticalSection CritSect;
	};
}