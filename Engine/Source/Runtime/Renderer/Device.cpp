#include "DrnPCH.h"
#include "Device.h"
#include "Runtime/Renderer/GpuFence.h"
#include "Runtime/Renderer/RenderResource.h"
#include "Runtime/Renderer/RenderBuffer.h"

LOG_DEFINE_CATEGORY( LogDevice, "Device" );

namespace Drn
{
	Device::Device()
		: m_DeferredDeletionQueue(this)
		, DefaultBufferAllocator(this)
		, DefaultFastAllocator(this, D3D12_HEAP_TYPE_UPLOAD, 1024 * 1024 * 4)
	{
		Microsoft::WRL::ComPtr<IDXGIFactory4> dxgiFactory;
		UINT createFactoryFlags = 0;
#if D3D12_DEBUG_LAYER
		createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif
		CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory));

		Microsoft::WRL::ComPtr<IDXGIAdapter1> dxgiAdapter1;
		Microsoft::WRL::ComPtr<IDXGIAdapter4> dxgiAdapter4;

		bool UseWarp = false;

		if (UseWarp)
		{
			dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&dxgiAdapter1));
			dxgiAdapter1.As(&dxgiAdapter4);
		}

		else
		{
			SIZE_T maxDedicatedVideoMemory = 0;
			for (UINT i = 0; dxgiFactory->EnumAdapters1(i, &dxgiAdapter1) != DXGI_ERROR_NOT_FOUND; ++i)
			{
				DXGI_ADAPTER_DESC1 dxgiAdapterDesc1;
				dxgiAdapter1->GetDesc1(&dxgiAdapterDesc1);

				if ((dxgiAdapterDesc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 &&
					SUCCEEDED(D3D12CreateDevice(dxgiAdapter1.Get(), 
						D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device), nullptr)) && 
					dxgiAdapterDesc1.DedicatedVideoMemory > maxDedicatedVideoMemory )
				{
					maxDedicatedVideoMemory = dxgiAdapterDesc1.DedicatedVideoMemory;
					dxgiAdapter1.As(&dxgiAdapter4);
				}
			}
		}

		D3D12CreateDevice(dxgiAdapter4.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(m_Device.GetAddressOf()));

		Microsoft::WRL::ComPtr<ID3D12InfoQueue> pInfoQueue;
		if (SUCCEEDED(m_Device.As(&pInfoQueue)))
		{
			//pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
			//pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
			//pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

			D3D12_MESSAGE_SEVERITY Severities[] =
			{
				D3D12_MESSAGE_SEVERITY_INFO
			};
 
			// Suppress individual messages by their ID
			D3D12_MESSAGE_ID DenyIds[] = {
				D3D12_MESSAGE_ID_COMMAND_LIST_DRAW_VERTEX_BUFFER_NOT_SET,
				D3D12_MESSAGE_ID_CREATERESOURCE_STATE_IGNORED, // https://stackoverflow.com/questions/77450713/why-am-i-getting-an-unhandled-exception-in-kernelbase-dll-when-im-trying-to-cal
			};
 
			D3D12_INFO_QUEUE_FILTER NewFilter = {};
			NewFilter.DenyList.NumSeverities = _countof(Severities);
			NewFilter.DenyList.pSeverityList = Severities;
			NewFilter.DenyList.NumIDs = _countof(DenyIds);
			NewFilter.DenyList.pIDList = DenyIds;
 
			pInfoQueue->PushStorageFilter(&NewFilter);
		}

#if WITH_EDITOR && 0
		Microsoft::WRL::ComPtr<ID3D12InfoQueue> pInfoQueue;
		if (SUCCEEDED(m_Device.As(&pInfoQueue)))
		{
			pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
			pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
			pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

			D3D12_MESSAGE_SEVERITY Severities[] =
			{
				D3D12_MESSAGE_SEVERITY_INFO
			};
 
			// Suppress individual messages by their ID
			D3D12_MESSAGE_ID DenyIds[] = {
				D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
				D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,                      
				D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,                    
			};
 
			D3D12_INFO_QUEUE_FILTER NewFilter = {};
			//NewFilter.DenyList.NumCategories = _countof(Categories);
			//NewFilter.DenyList.pCategoryList = Categories;
			NewFilter.DenyList.NumSeverities = _countof(Severities);
			NewFilter.DenyList.pSeverityList = Severities;
			NewFilter.DenyList.NumIDs = _countof(DenyIds);
			NewFilter.DenyList.pIDList = DenyIds;
 
			pInfoQueue->PushStorageFilter(&NewFilter);
		}
#endif

#if D3D12_Debug_INFO
		m_Device->SetName(L"MainDevice");
#endif

		dxgiAdapter4->GetDesc3(&m_Description);
		LOG( LogDevice, Info, "successfully created device %ws", m_Description.Description );
	}

	Device::~Device()
	{
		//m_DeferredDeletionQueue.ReleaseResources();
		//DefaultBufferAllocator.FreeDefaultBufferPools();
		//DefaultFastAllocator.Destroy();

		LOG( LogDevice, Info, "removing device %ws", m_Description.Description );
	}


	void Device::CreateCommittedResource( const D3D12_RESOURCE_DESC& InDesc, const D3D12_HEAP_PROPERTIES& HeapProps, D3D12_RESOURCE_STATES InInitialState, bool bNeedsStateTracking,
		const D3D12_CLEAR_VALUE* ClearValue, RenderResource** ppOutResource, const std::string& Name )
	{
		drn_check(ppOutResource);

		TRefCountPtr<ID3D12Resource> pResource;

		const bool bRequiresInitialization = (InDesc.Flags & (D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)) != 0;
		D3D12_HEAP_FLAGS HeapFlags = !bRequiresInitialization ? D3D12_HEAP_FLAG_CREATE_NOT_ZEROED : D3D12_HEAP_FLAG_NONE;

		const HRESULT hr = m_Device->CreateCommittedResource(&HeapProps, HeapFlags, &InDesc, InInitialState, ClearValue, IID_PPV_ARGS(pResource.GetInitReference()));

		if (SUCCEEDED(hr))
		{
			*ppOutResource = new RenderResource(this, pResource, InDesc, InInitialState, bNeedsStateTracking, HeapProps.Type);
			(*ppOutResource)->AddRef();

			SetName(*ppOutResource, Name);

			//if (IsCPUInaccessible(HeapProps.Type))
			//{
			//	(*ppOutResource)->StartTrackingForResidency();
			//}
		}
	}

	void Device::CreateBuffer( D3D12_HEAP_TYPE HeapType, uint64 Size, D3D12_RESOURCE_STATES InInitialState,
		bool bNeedsStateTracking, class RenderResource** ppOutResource, const std::string& Name, D3D12_RESOURCE_FLAGS Flags )
	{
		drn_check(ppOutResource);

		const D3D12_RESOURCE_DESC BufferDesc = CD3DX12_RESOURCE_DESC::Buffer(Size, Flags);
		return CreateCommittedResource(BufferDesc, CD3DX12_HEAP_PROPERTIES(HeapType), InInitialState, bNeedsStateTracking, nullptr, ppOutResource, Name);
	}

	void Device::AllocateBuffer( const D3D12_RESOURCE_DESC& InDesc, uint32 Size, uint32 InUsage, bool bNeedsStateTracking,
		RenderResourceCreateInfo& CreateInfo, uint32 Alignment, ResourceLocation& Location )
	{
		drn_check(Size > 0);

		const bool bIsDynamic = InUsage & (uint32)EBufferUsageFlags::AnyDynamic;

		//if (bIsDynamic)
		//{
		//	//drn_check(!bNeedsStateTracking);
		//	void* pData = GetUploadHeapAllocator(Device->GetGPUIndex()).AllocUploadResource(Size, Alignment, Location);
		//	drn_check(Location.GetSize() == Size);
		//
		//	if (CreateInfo.ResourceArray)
		//	{
		//		const void* InitialData = CreateInfo.ResourceArray->GetResourceData();
		//
		//		check(Size == CreateInfo.ResourceArray->GetResourceDataSize());
		//		// Handle initial data
		//		FMemory::Memcpy(pData, InitialData, Size);
		//	}
		//}
		//else
		{
			GetDefaultBufferAllocator().AllocDefaultResource(D3D12_HEAP_TYPE_DEFAULT, InDesc, (EBufferUsageFlags)InUsage, bNeedsStateTracking, Location, Alignment, CreateInfo.DebugName);
			drn_check(Location.GetSize() == Size);
		}
	}

	template<typename BufferType>
	BufferType* Device::CreateRenderBuffer( D3D12CommandList* CmdList, const D3D12_RESOURCE_DESC& Desc, uint32 Alignment, uint32 Stride, uint32 Size, uint32 InUsage,
		bool bNeedsStateTracking, RenderResourceCreateInfo& CreateInfo )
	{
		BufferType* BufferOut = nullptr;

		const bool bIsDynamic = InUsage & (uint32)EBufferUsageFlags::AnyDynamic;
		//if (bIsDynamic)
		//{
		//	BufferType* NewBuffer0 = nullptr;
		//	BufferOut = CreateLinkedObject<BufferType>(CreateInfo.GPUMask, [&](FD3D12Device* Device)
		//	{
		//		BufferType* NewBuffer = new BufferType(Device, Stride, Size, InUsage);
		//		NewBuffer->BufferAlignment = Alignment;
		//
		//		if (Device->GetGPUIndex() == FirstGPUIndex)
		//		{
		//			AllocateBuffer(Device, InDesc, Size, InUsage, InResourceStateMode, CreateInfo, Alignment, *NewBuffer, NewBuffer->ResourceLocation);
		//			NewBuffer0 = NewBuffer;
		//		}
		//		else
		//		{
		//			check(NewBuffer0);
		//			FD3D12ResourceLocation::ReferenceNode(Device, NewBuffer->ResourceLocation, NewBuffer0->ResourceLocation);
		//		}
		//
		//		return NewBuffer;
		//	});
		//}
		//else
		{
			BufferOut = new BufferType(this, Stride, Size, InUsage);
			BufferOut->BufferAlignment = Alignment;

			AllocateBuffer(Desc, Size, InUsage, bNeedsStateTracking, CreateInfo, Alignment, BufferOut->m_ResourceLocation);
		}

		if (CreateInfo.ResourceArray)
		{
			if (bIsDynamic == false && BufferOut->m_ResourceLocation.IsValid())
			{
				//drn_check(Size == CreateInfo.ResourceArray->GetResourceDataSize());
		
				ResourceLocation SrcResourceLoc(BufferOut->GetParentDevice());
				void* pData;
				//const bool bOnAsyncThread = !IsInRHIThread() && !IsInRenderingThread();
				//
				//// Get an upload heap and initialize data
				//if (bOnAsyncThread)
				//{
				//	const uint32 GPUIdx = SrcResourceLoc.GetParentDevice()->GetGPUIndex();
				//	pData = GetUploadHeapAllocator(GPUIdx).AllocUploadResource(Size, 4u, SrcResourceLoc);
				//}
				//else
				{
					pData = SrcResourceLoc.GetParentDevice()->GetDefaultFastAllocator().Allocate(Size, 4UL, &SrcResourceLoc);
				}
				drn_check(pData);
				//FMemory::Memcpy(pData, CreateInfo.ResourceArray->GetResourceData(), Size);
				memcpy(pData, CreateInfo.ResourceArray, Size);

				//if (bOnAsyncThread)
				//{
				//	// Need to update buffer content on RHI thread (immediate context) because the buffer can be a
				//	// sub-allocation and its backing resource may be in a state incompatible with the copy queue.
				//	// TODO:
				//	// Create static buffers in COMMON state, rely on state promotion/decay to avoid transition barriers,
				//	// and initialize them asynchronously on the copy queue. D3D12 buffers always allow simultaneous acess
				//	// so it is legal to write to a region on the copy queue while other non-overlapping regions are
				//	// being read on the graphics/compute queue. Currently, d3ddebug throws error for such usage.
				//	// Once Microsoft (via Windows update) fix the debug layer, async static buffer initialization should
				//	// be done on the copy queue.
				//	FD3D12ResourceLocation* SrcResourceLoc_Heap = new FD3D12ResourceLocation(SrcResourceLoc.GetParentDevice());
				//	FD3D12ResourceLocation::TransferOwnership(*SrcResourceLoc_Heap, SrcResourceLoc);
				//	ENQUEUE_RENDER_COMMAND(CmdD3D12InitializeBuffer)(
				//		[BufferOut, SrcResourceLoc_Heap, Size](FRHICommandListImmediate& RHICmdList)
				//	{
				//		if (RHICmdList.Bypass())
				//		{
				//			FD3D12RHICommandInitializeBuffer Command(BufferOut, *SrcResourceLoc_Heap, Size);
				//			Command.ExecuteNoCmdList();
				//		}
				//		else
				//		{
				//			new (RHICmdList.AllocCommand<FD3D12RHICommandInitializeBuffer>()) FD3D12RHICommandInitializeBuffer(BufferOut, *SrcResourceLoc_Heap, Size);
				//		}
				//		delete SrcResourceLoc_Heap;
				//	});
				//}
				//else if (!RHICmdList || RHICmdList->Bypass())
				//{
				//	// On RHIT or RT (when bypassing), we can access immediate context directly
				//	FD3D12RHICommandInitializeBuffer Command(BufferOut, SrcResourceLoc, Size);
				//	Command.ExecuteNoCmdList();
				//}
				//else
				{
					RenderResource* Destination = BufferOut->m_ResourceLocation.GetResource();
					Device* Device = Destination->GetParentDevice();

					{
						// Writable structured buffers are sometimes initialized with initial data which means they sometimes need tracking.
						ConditionalScopeResourceBarrier ConditionalScopeResourceBarrier(CmdList, Destination, D3D12_RESOURCE_STATE_COPY_DEST, 0);
						CmdList->FlushBarriers();

						//CommandContext.numInitialResourceCopies++;

						uint64 Start = SrcResourceLoc.GetOffsetFromBaseOfResource();
						uint64 End = BufferOut->m_ResourceLocation.GetOffsetFromBaseOfResource();

						CmdList->GetD3D12CommandList()->CopyBufferRegion( Destination->GetResource(), BufferOut->m_ResourceLocation.GetOffsetFromBaseOfResource(),
							SrcResourceLoc.GetResource()->GetResource(), SrcResourceLoc.GetOffsetFromBaseOfResource(), Size);

						//hCommandList.UpdateResidency(Destination);
						//hCommandList.UpdateResidency(SrcResourceLoc.GetResource());

						//CommandContext.ConditionalFlushCommandList();
					}
				}
			}
		
			// Discard the resource array's contents.
			//CreateInfo.ResourceArray->Discard();
		}

		UpdateBufferStats<BufferType>(&BufferOut->m_ResourceLocation, true);

		return BufferOut;
	}

	template RenderIndexBuffer* Device::CreateRenderBuffer<RenderIndexBuffer>( D3D12CommandList* CmdList, const D3D12_RESOURCE_DESC& Desc, uint32 Alignment,
		uint32 Stride, uint32 Size, uint32 InUsage, bool bNeedsStateTracking, RenderResourceCreateInfo& CreateInfo );

	template<>
	void Device::UpdateBufferStats<RenderIndexBuffer>( ResourceLocation* Location, bool bAllocating )
	{
		BufferStats::UpdateBufferStats(Location, true, BufferStats::EBufferMemoryStatGroups::Index);
	}

// -----------------------------------------------------------------------------------------------------------

	DeferredDeletionQueue::DeferredDeletionQueue( class Device* InParent )
		: DeviceChild(InParent)
	{}

	DeferredDeletionQueue::~DeferredDeletionQueue()
	{}

	void DeferredDeletionQueue::EnqueueResource( ID3D12Object* pResource, class GpuFence* Fence )
	{
		FencedObjectType FencedObject;
		FencedObject.D3DObject = pResource;
		FencedObject.Type = EObjectType::D3D;
		FencedObject.FencePair = FencePair(Fence, Fence->GetCurrentFence());
		DeferredReleaseQueue.push(FencedObject);
	}

	void DeferredDeletionQueue::EnqueueResource( RenderResource* pResource, class GpuFence* Fence )
	{
		drn_check(pResource->ShouldDeferDelete());

		FencedObjectType FencedObject;
		FencedObject.RHIObject = pResource;
		FencedObject.Type = EObjectType::RHI;
		FencedObject.FencePair = FencePair(Fence, Fence->GetCurrentFence());
		DeferredReleaseQueue.push(FencedObject);
	}

	void DeferredDeletionQueue::ReleaseCompletedResources()
	{
		FencedObjectType FencedObject;
		while (!DeferredReleaseQueue.empty())
		{
			FencedObject = DeferredReleaseQueue.front();
			GpuFence* Fence = FencedObject.FencePair.first;
			uint64 SignaledValue = FencedObject.FencePair.second;

			if (!Fence->IsFenceComplete(SignaledValue))
			{
				return;
			}

			if (FencedObject.Type == EObjectType::RHI)
			{
				drn_check(FencedObject.RHIObject->GetRefCount() == 1);
				FencedObject.RHIObject->Release();
			}
			else
			{
				FencedObject.D3DObject->Release();
			}

			DeferredReleaseQueue.pop();
		}
	}

	void DeferredDeletionQueue::ReleaseResources()
	{
		LOG(LogDevice, Info, "D3D12 ReleaseResources: %zu items to release", DeferredReleaseQueue.size());

		FencedObjectType FencedObject;
		while (!DeferredReleaseQueue.empty())
		{
			FencedObject = DeferredReleaseQueue.front();
			if (FencedObject.Type == EObjectType::RHI)
			{
				const D3D12_RESOURCE_DESC Desc = FencedObject.RHIObject->GetDesc();
				LOG(LogDevice, Info, "D3D12 ReleaseResources: \"%s\", %llu x %u x %u, Mips: %u, Format: 0x%X, Flags: 0x%X", FencedObject.RHIObject->GetName().c_str(), Desc.Width, Desc.Height, Desc.DepthOrArraySize, Desc.MipLevels, Desc.Format, Desc.Flags);

				uint32 RefCount = FencedObject.RHIObject->Release();
				if (RefCount)
				{
					LOG(LogDevice, Info, TEXT("RefCount was %u"), RefCount);
				}
			}

			else
			{
				const uint32 RefCount = FencedObject.D3DObject->Release();
				if (RefCount)
				{
					LOG(LogDevice, Info, "RefCount was %u", RefCount);
				}
			}

			DeferredReleaseQueue.pop();
		}
	}
}