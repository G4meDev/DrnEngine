#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class Device;

	class OfflineDescriptorManager : public DeviceChild
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

		static D3D12_DESCRIPTOR_HEAP_DESC CreateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE Type, uint32 NumDescriptorsPerHeap)
		{
			D3D12_DESCRIPTOR_HEAP_DESC Desc = {};
			Desc.Type = Type;
			Desc.NumDescriptors = NumDescriptorsPerHeap;
			Desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

			return Desc;
		}
	
	public:
		OfflineDescriptorManager(Device* Parent, D3D12_DESCRIPTOR_HEAP_TYPE Type, uint32 NumDescriptorsPerHeap)
			: DeviceChild(Parent)
			, m_Desc(CreateDescriptor(Type, NumDescriptorsPerHeap))
			, m_DescriptorSize(0)
		{}

		virtual ~OfflineDescriptorManager();

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

	class OnlineDescriptorManager : public DeviceChild
	{
	public:
		typedef D3D12_CPU_DESCRIPTOR_HANDLE HeapOffset;
		typedef decltype(HeapOffset::ptr) HeapOffsetRaw;
		typedef uint32 HeapIndex;
	
	private:
		struct SFreeRange { HeapOffsetRaw Start; HeapOffsetRaw End; };
		struct SHeapEntry
		{
			std::list<SFreeRange> m_FreeList;

			SHeapEntry() { }
		};
		typedef std::vector<SHeapEntry> THeapMap;

		static D3D12_DESCRIPTOR_HEAP_DESC CreateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE Type, uint32 NumDescriptorsPerHeap)
		{
			D3D12_DESCRIPTOR_HEAP_DESC Desc = {};
			Desc.Type = Type;
			Desc.NumDescriptors = NumDescriptorsPerHeap;
			Desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

			return Desc;
		}
	
	public:
		OnlineDescriptorManager(Device* Parent, D3D12_DESCRIPTOR_HEAP_TYPE Type, uint32 NumDescriptorsPerChunk, uint32 NumChunks)
			: DeviceChild(Parent)
			, m_NumChunks(NumChunks)
			, m_NumDescriptorsPerChunk(NumDescriptorsPerChunk)
			, m_Desc(CreateDescriptor(Type, NumDescriptorsPerChunk * NumChunks))
			, m_DescriptorSize(0)
		{}

		virtual ~OnlineDescriptorManager();

		ID3D12DescriptorHeap* GetHeap() const { return m_Heap; }

		void Init();

		HeapOffset AllocateHeapSlot( HeapIndex& OutHeapIndex, uint64& GpuHandle, uint32& Index);
		void FreeHeapSlot( HeapOffset Offset, HeapIndex index );

		int32 GetNumAllocatedHandles();

	private:
		TRefCountPtr<ID3D12DescriptorHeap> m_Heap;
		const D3D12_DESCRIPTOR_HEAP_DESC m_Desc;
		uint32 m_DescriptorSize;
		uint32 m_NumChunks;
		uint32 m_NumDescriptorsPerChunk;

		THeapMap m_Heaps;
		std::list<HeapIndex> m_FreeHeaps;
		CriticalSection CritSect;
	};
}