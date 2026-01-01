#include "DrnPCH.h"
#include "Descriptor.h"

namespace Drn
{
	OfflineDescriptorManager::~OfflineDescriptorManager()
	{
		//for (auto Heap : m_Heaps)
		//{
		//	drn_check(Heap.m_FreeList.size() == 1);
		//
		//	SFreeRange& Range = Heap.m_FreeList.front();
		//	drn_check(Range.Start + m_DescriptorSize * m_Desc.NumDescriptors == Range.End);
		//}
	}

	void OfflineDescriptorManager::Init()
	{
		m_DescriptorSize = Parent->GetD3D12Device()->GetDescriptorHandleIncrementSize(m_Desc.Type);
	}

	OfflineDescriptorManager::HeapOffset OfflineDescriptorManager::AllocateHeapSlot( HeapIndex& outIndex )
	{
		ScopeLock Lock(&CritSect);
		if (0 == m_FreeHeaps.size())
		{
			AllocateHeap();
		}
		drn_check(0 != m_FreeHeaps.size());
		HeapIndex Head = m_FreeHeaps.front();
		outIndex = Head;
		SHeapEntry &HeapEntry = m_Heaps[outIndex];
		drn_check(0 != HeapEntry.m_FreeList.size());
		SFreeRange &Range = HeapEntry.m_FreeList.front();
		HeapOffset Ret = { Range.Start };
		Range.Start += m_DescriptorSize;

		if (Range.Start == Range.End)
		{
			HeapEntry.m_FreeList.erase(HeapEntry.m_FreeList.begin());
			if (0 == HeapEntry.m_FreeList.size())
			{
				m_FreeHeaps.erase(m_FreeHeaps.begin());
			}
		}
		return Ret;
	}

	void OfflineDescriptorManager::FreeHeapSlot( HeapOffset Offset, HeapIndex index )
	{
		ScopeLock Lock(&CritSect);
		SHeapEntry &HeapEntry = m_Heaps[index];

		SFreeRange NewRange =
		{
			Offset.ptr,
			Offset.ptr + m_DescriptorSize
		};

		bool bFound = false;
		for (auto Node = HeapEntry.m_FreeList.begin(); Node != HeapEntry.m_FreeList.end() && !bFound; Node++)
		{
			SFreeRange &Range = *Node;
			drn_check(Range.Start < Range.End);
			if (Range.Start == Offset.ptr + m_DescriptorSize)
			{
				Range.Start = Offset.ptr;
				bFound = true;
			}
			else if (Range.End == Offset.ptr)
			{
				Range.End += m_DescriptorSize;
				bFound = true;
			}
			else
			{
				drn_check(Range.End < Offset.ptr || Range.Start > Offset.ptr);
				if (Range.Start > Offset.ptr)
				{
					HeapEntry.m_FreeList.insert(Node, NewRange);
					bFound = true;
				}
			}
		}

		if (!bFound)
		{
			if (0 == HeapEntry.m_FreeList.size())
			{
				m_FreeHeaps.push_back(index);
			}
			HeapEntry.m_FreeList.push_back(NewRange);
		}
	}

	void OfflineDescriptorManager::AllocateHeap()
	{
		TRefCountPtr<ID3D12DescriptorHeap> Heap;
		GetParentDevice()->GetD3D12Device()->CreateDescriptorHeap(&m_Desc, IID_PPV_ARGS(Heap.GetInitReference()));
		SetName(Heap, "Offline Descriptor Heap");

		HeapOffset HeapBase = Heap->GetCPUDescriptorHandleForHeapStart();
		drn_check(HeapBase.ptr != 0);

		m_Heaps.push_back({});
		SHeapEntry& HeapEntry = m_Heaps.back();
		HeapEntry.m_FreeList.push_back({ HeapBase.ptr, HeapBase.ptr + m_Desc.NumDescriptors * m_DescriptorSize });
		HeapEntry.m_Heap = Heap;
		m_FreeHeaps.push_back(m_Heaps.size() - 1);
	}

	int32 OfflineDescriptorManager::GetNumAllocatedHandles()
	{
		int32 Result = 0;

		for (const SHeapEntry& Heap : m_Heaps)
		{
			int32 TotalFreeRange = 0;
			for (const SFreeRange& FreeRange : Heap.m_FreeList)
			{
				TotalFreeRange += FreeRange.End - FreeRange.Start;
			}

			Result += m_Desc.NumDescriptors - (TotalFreeRange / m_DescriptorSize);
		}

		return Result;
	}


	OnlineDescriptorManager::~OnlineDescriptorManager()
	{
		//for (auto Heap : m_Heaps)
		//{
		//	drn_check(Heap.m_FreeList.size() == 1);
		//
		//	SFreeRange& Range = Heap.m_FreeList.front();
		//	drn_check(Range.Start + m_DescriptorSize * m_Desc.NumDescriptors == Range.End);
		//}
	}

	void OnlineDescriptorManager::Init()
	{
		m_DescriptorSize = Parent->GetD3D12Device()->GetDescriptorHandleIncrementSize(m_Desc.Type);

		GetParentDevice()->GetD3D12Device()->CreateDescriptorHeap(&m_Desc, IID_PPV_ARGS(m_Heap.GetInitReference()));
		SetName(m_Heap, "Online Descriptor Heap");

		m_Heaps.resize(m_NumChunks);

		for (int32 i = 0; i < m_NumChunks; i++)
		{
			const HeapOffsetRaw Start = m_Heap->GetCPUDescriptorHandleForHeapStart().ptr + i * m_NumDescriptorsPerChunk * m_DescriptorSize;
			const HeapOffsetRaw End = Start + m_NumDescriptorsPerChunk * m_DescriptorSize;

			m_Heaps[i].m_FreeList.push_back({Start, End});
			m_FreeHeaps.push_back(i);
		}
	}

	OnlineDescriptorManager::HeapOffset OnlineDescriptorManager::AllocateHeapSlot(HeapIndex& OutHeapIndex, uint64& GpuHandle, uint32& Index)
	{
		drn_check(!m_FreeHeaps.empty());
		ScopeLock Lock(&CritSect);

		HeapIndex Head = m_FreeHeaps.front();
		OutHeapIndex = Head;
		SHeapEntry &HeapEntry = m_Heaps[OutHeapIndex];
		drn_check(0 != HeapEntry.m_FreeList.size());

		SFreeRange &Range = HeapEntry.m_FreeList.front();
		HeapOffset Ret = { Range.Start };
		Range.Start += m_DescriptorSize;

		if (Range.Start == Range.End)
		{
			HeapEntry.m_FreeList.erase(HeapEntry.m_FreeList.begin());
			if (0 == HeapEntry.m_FreeList.size())
			{
				m_FreeHeaps.erase(m_FreeHeaps.begin());
			}
		}

		Index = (Ret.ptr - m_Heap->GetCPUDescriptorHandleForHeapStart().ptr) / m_DescriptorSize;
		GpuHandle = m_Heap->GetGPUDescriptorHandleForHeapStart().ptr + Index * m_DescriptorSize;

		return Ret;
	}

	void OnlineDescriptorManager::FreeHeapSlot( HeapOffset Offset, HeapIndex index )
	{
		ScopeLock Lock(&CritSect);
		SHeapEntry &HeapEntry = m_Heaps[index];

		SFreeRange NewRange =
		{
			Offset.ptr,
			Offset.ptr + m_DescriptorSize
		};

		bool bFound = false;
		for (auto Node = HeapEntry.m_FreeList.begin(); Node != HeapEntry.m_FreeList.end() && !bFound; Node++)
		{
			SFreeRange &Range = *Node;
			drn_check(Range.Start < Range.End);
			if (Range.Start == Offset.ptr + m_DescriptorSize)
			{
				Range.Start = Offset.ptr;
				bFound = true;
			}
			else if (Range.End == Offset.ptr)
			{
				Range.End += m_DescriptorSize;
				bFound = true;
			}
			else
			{
				drn_check(Range.End < Offset.ptr || Range.Start > Offset.ptr);
				if (Range.Start > Offset.ptr)
				{
					HeapEntry.m_FreeList.insert(Node, NewRange);
					bFound = true;
				}
			}
		}

		if (!bFound)
		{
			if (0 == HeapEntry.m_FreeList.size())
			{
				m_FreeHeaps.push_back(index);
			}
			HeapEntry.m_FreeList.push_back(NewRange);
		}
	}

	int32 OnlineDescriptorManager::GetNumAllocatedHandles()
	{
		int32 Result = 0;

		for (const SHeapEntry& Heap : m_Heaps)
		{
			int32 TotalFreeRange = 0;
			for (const SFreeRange& FreeRange : Heap.m_FreeList)
			{
				TotalFreeRange += FreeRange.End - FreeRange.Start;
			}

			Result += m_NumDescriptorsPerChunk - (TotalFreeRange / m_DescriptorSize);
		}

		return Result;
	}

        }  // namespace Drn