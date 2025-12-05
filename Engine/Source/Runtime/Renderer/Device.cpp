#include "DrnPCH.h"
#include "Device.h"
#include "Runtime/Renderer/GpuFence.h"

LOG_DEFINE_CATEGORY( LogDevice, "Device" );

namespace Drn
{
	Device::Device()
		: m_DeferredDeletionQueue(this)
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
				D3D12_MESSAGE_ID_COMMAND_LIST_DRAW_VERTEX_BUFFER_NOT_SET
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
		m_DeferredDeletionQueue.ReleaseResources();

		LOG( LogDevice, Info, "removing device %ws", m_Description.Description );
	}


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
			if (!FencedObject.FencePair.first->IsFenceComplete(FencedObject.FencePair.second));
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