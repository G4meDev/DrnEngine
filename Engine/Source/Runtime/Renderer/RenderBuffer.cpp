#include "DrnPCH.h"
#include "RenderBuffer.h"

namespace Drn
{
	D3D12_RESOURCE_DESC CreateIndexBufferResourceDesc(uint32 Size, uint32 InUsage)
	{
		D3D12_RESOURCE_DESC Desc = CD3DX12_RESOURCE_DESC::Buffer(Size);

		if (InUsage & (uint32)EBufferUsageFlags::UnorderedAccess)
		{
			Desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		}

		if ((InUsage & (uint32)EBufferUsageFlags::ShaderResource) == 0)
		{
			Desc.Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
		}

		return Desc;
	}

	D3D12_RESOURCE_DESC CreateVertexBufferResourceDesc(uint32 Size, uint32 InUsage)
	{
		D3D12_RESOURCE_DESC Desc = CD3DX12_RESOURCE_DESC::Buffer(Size);

		if (InUsage & (uint32)EBufferUsageFlags::UnorderedAccess)
		{
			Desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

			//static bool bRequiresRawView = (GMaxRHIFeatureLevel < ERHIFeatureLevel::SM5);
			//if (bRequiresRawView)
			//{
			//	// Force the buffer to be a raw, byte address buffer
			//	InUsage |= BUF_ByteAddressBuffer;
			//}
		}

		if ((InUsage & (uint32)EBufferUsageFlags::ShaderResource) == 0)
		{
			Desc.Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
		}

		return Desc;
	}

	D3D12_RESOURCE_DESC CreateStructuredBufferResourceDesc(uint32 Size, uint32 InUsage)
	{
		D3D12_RESOURCE_DESC Desc = CD3DX12_RESOURCE_DESC::Buffer(Size);

		if (InUsage & (uint32)EBufferUsageFlags::UnorderedAccess)
		{
			Desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		}

		if ((InUsage & (uint32)EBufferUsageFlags::ShaderResource) == 0)
		{
			Desc.Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
		}

		return Desc;
	}

	D3D12_RESOURCE_DESC CreateRawBufferResourceDesc(uint32 Size, uint32 InUsage)
	{
		D3D12_RESOURCE_DESC Desc = CD3DX12_RESOURCE_DESC::Buffer(Size);

		if (InUsage & (uint32)EBufferUsageFlags::UnorderedAccess)
		{
			Desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		}

		if ((InUsage & (uint32)EBufferUsageFlags::ShaderResource) == 0)
		{
			Desc.Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
		}

		return Desc;
	}

	void RenderBufferBase::ReleaseUnderlyingResource()
	{
		m_ResourceLocation.Clear();
		RemoveAllDynamicSRVs();
	}

	RenderIndexBuffer::~RenderIndexBuffer()
	{
		if (m_ResourceLocation.IsValid())
		{
			BufferStats::UpdateBufferStats(&m_ResourceLocation, false, BufferStats::EBufferMemoryStatGroups::Index);
		}
	}

	void RenderIndexBuffer::ReleaseUnderlyingResource()
	{
		BufferStats::UpdateBufferStats(&m_ResourceLocation, false, BufferStats::EBufferMemoryStatGroups::Index);
		RenderBufferBase::ReleaseUnderlyingResource();
		Stride = Size = Usage = 0;
	}

	RenderIndexBuffer* RenderIndexBuffer::Create(Device* InParent, D3D12CommandList* CmdList, uint32 Stride, uint32 Size, uint32 InUsage, D3D12_RESOURCE_STATES InResourceState, bool bNeedsStateTracking, RenderResourceCreateInfo& CreateInfo )
	{
		const D3D12_RESOURCE_DESC Desc = CreateIndexBufferResourceDesc(Size, InUsage);
		const uint32 Alignment = 4;

		RenderIndexBuffer* Buffer = InParent->CreateRenderBuffer<RenderIndexBuffer>(CmdList, Desc, Alignment, Stride, Size, InUsage, bNeedsStateTracking, CreateInfo);
		//if (Buffer->ResourceLocation.IsTransient())
		//{
		//	// TODO: this should ideally be set in platform-independent code, since this tracking is for the high level
		//	Buffer->SetCommitted(false);
		//}

		return Buffer;
	}

// --------------------------------------------------------------------------------------------------------

	RenderVertexBuffer::~RenderVertexBuffer()
	{
		if (m_ResourceLocation.IsValid())
		{
			BufferStats::UpdateBufferStats(&m_ResourceLocation, false, BufferStats::EBufferMemoryStatGroups::Vertex);
		}
	}

	void RenderVertexBuffer::ReleaseUnderlyingResource()
	{
		BufferStats::UpdateBufferStats(&m_ResourceLocation, false, BufferStats::EBufferMemoryStatGroups::Vertex);
		RenderBufferBase::ReleaseUnderlyingResource();
		Size = Usage = 0;
	}

	RenderVertexBuffer* RenderVertexBuffer::Create( class Device* InParent, D3D12CommandList* CmdList, uint32 Size, uint32 InUsage,
		D3D12_RESOURCE_STATES InResourceState, bool bNeedsStateTracking, RenderResourceCreateInfo& CreateInfo )
	{
		const D3D12_RESOURCE_DESC Desc = CreateVertexBufferResourceDesc( Size, InUsage );
		const uint32 Alignment = 4;

		RenderVertexBuffer* Buffer = InParent->CreateRenderBuffer<RenderVertexBuffer>(
			CmdList, Desc, Alignment, 0, Size, InUsage, bNeedsStateTracking, CreateInfo );
		// if (Buffer->ResourceLocation.IsTransient())
		//{
		//	// TODO: this should ideally be set in platform-independent code, since this tracking is for the
		//high level 	Buffer->SetCommitted(false);
		// }

		return Buffer;
	}

// --------------------------------------------------------------------------------------------------------

	RenderUniformBuffer::~RenderUniformBuffer()
	{
		BufferStats::UpdateBufferStats(&ResourceLocation, false, BufferStats::EBufferMemoryStatGroups::Constant);
		delete View;
	}

	RenderUniformBuffer* RenderUniformBuffer::Create( Device* InParent, uint32 InSize, EUniformBufferUsage Usage, const void* Contents )
	{
		drn_check(InSize > 0);
		RenderUniformBuffer* NewUniformBuffer = new RenderUniformBuffer(InParent, InSize, Usage);

		//drn_check(Align(InSize, 16) == InSize);
		drn_check(InSize <= D3D12_REQ_CONSTANT_BUFFER_ELEMENT_COUNT * 16);

		const uint32 AlignedSize = Align( InSize, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT );
		NewUniformBuffer->View = new ConstantBufferView();

		void* MappedData = nullptr;
		//if (Usage == EUniformBufferUsage::MultiFrame)
		//{
		//	// Uniform buffers that live for multiple frames must use the more expensive and persistent allocation path
		//	FD3D12DynamicHeapAllocator& Allocator = GetAdapter().GetUploadHeapAllocator(Device->GetGPUIndex());
		//	MappedData = Allocator.AllocUploadResource(NumBytesActualData, DEFAULT_CONTEXT_UPLOAD_POOL_ALIGNMENT, NewUniformBuffer->ResourceLocation);
		//}

		if (Usage == EUniformBufferUsage::MultiFrame)
		{
			DynamicHeapAllocator& Allocator = InParent->GetDynamicHeapAllocator();
			MappedData = Allocator.AllocUploadResource(InSize, 256, NewUniformBuffer->ResourceLocation);
		}
		else
		{
			FastConstantAllocator& Allocator = InParent->GetTransientUniformBufferAllocator();

			MappedData = Allocator.Allocate(AlignedSize, NewUniformBuffer->ResourceLocation);
		}
		drn_check(NewUniformBuffer->ResourceLocation.GetOffsetFromBaseOfResource() % 16 == 0);

		drn_check(MappedData != nullptr);
		memcpy(MappedData, Contents, InSize);

		NewUniformBuffer->View->Create(NewUniformBuffer->ResourceLocation.GetGPUVirtualAddress(), AlignedSize);

		BufferStats::UpdateBufferStats(&NewUniformBuffer->ResourceLocation, true, BufferStats::EBufferMemoryStatGroups::Constant);

		return NewUniformBuffer;
	}

	uint32 RenderUniformBuffer::GetViewIndex() const
	{
		return View ? View->GetIndex() : 0;
	}

// --------------------------------------------------------------------------------------------------------

	RenderStructuredBuffer::~RenderStructuredBuffer()
	{
		if (m_ResourceLocation.IsValid())
		{
			BufferStats::UpdateBufferStats(&m_ResourceLocation, false, BufferStats::EBufferMemoryStatGroups::Structured);
		}
	}

	void RenderStructuredBuffer::ReleaseUnderlyingResource()
	{
		BufferStats::UpdateBufferStats(&m_ResourceLocation, false, BufferStats::EBufferMemoryStatGroups::Structured);
		RenderBufferBase::ReleaseUnderlyingResource();
		Stride = Size = Usage = 0;
	}

	RenderStructuredBuffer* RenderStructuredBuffer::Create(Device* InParent, D3D12CommandList* CmdList, uint32 Stride, uint32 Size, uint32 InUsage,
		D3D12_RESOURCE_STATES InResourceState, bool bNeedsStateTracking, RenderResourceCreateInfo& CreateInfo)
	{
		drn_check(Size / Stride > 0 && Size % Stride == 0);

		const D3D12_RESOURCE_DESC ResourceDesc = CreateStructuredBufferResourceDesc( Size, InUsage );
		const uint32 Alignment = Stride;

		RenderStructuredBuffer* Buffer = InParent->CreateRenderBuffer<RenderStructuredBuffer>(
			CmdList, ResourceDesc, Alignment, 0, Size, InUsage, bNeedsStateTracking, CreateInfo );

		if ((InUsage & (uint32)EBufferUsageFlags::ShaderResource))
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC Desc;
			Desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			Desc.Format = DXGI_FORMAT_UNKNOWN;
			Desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			Desc.Buffer.StructureByteStride = Stride;
			Desc.Buffer.FirstElement = 0;
			Desc.Buffer.NumElements = Size / Stride;
			Desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
			Buffer->SRV = new ShaderResourceView(InParent, Desc, Buffer->m_ResourceLocation, Stride);
		}
		
		if ((InUsage & (uint32)EBufferUsageFlags::UnorderedAccess))
		{
			D3D12_UNORDERED_ACCESS_VIEW_DESC Desc;
			Desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
			Desc.Format = DXGI_FORMAT_UNKNOWN;
			Desc.Buffer.CounterOffsetInBytes = 0;
			Desc.Buffer.FirstElement = 0;
			Desc.Buffer.StructureByteStride = Stride;
			Desc.Buffer.NumElements = Size / Stride;
			Desc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

			Buffer->UAV = new UnorderedAccessView(InParent, Desc, Buffer->m_ResourceLocation);
		}

		return Buffer;
	}

	uint32 RenderStructuredBuffer::GetSrvIndex() const
	{
		return SRV ? SRV->GetDescriptorHeapIndex() : 0;
	}

	uint32 RenderStructuredBuffer::GetUavIndex() const
	{
		return UAV ? UAV->GetDescriptorHeapIndex() : 0;
	}

// --------------------------------------------------------------------------------------------------------

	RenderRawBuffer::~RenderRawBuffer()
	{
		if (m_ResourceLocation.IsValid())
		{
			BufferStats::UpdateBufferStats(&m_ResourceLocation, false, BufferStats::EBufferMemoryStatGroups::Buffer);
		}
	}

	void RenderRawBuffer::ReleaseUnderlyingResource()
	{
		BufferStats::UpdateBufferStats(&m_ResourceLocation, false, BufferStats::EBufferMemoryStatGroups::Buffer);
		RenderBufferBase::ReleaseUnderlyingResource();
		NumElements = BytesPerElement = Size = Usage = 0;
	}

	RenderRawBuffer* RenderRawBuffer::Create(Device* InParent, D3D12CommandList* CmdList, uint32 BytesPerElement, uint32 NumElements, DXGI_FORMAT Foramt, uint32 InUsage,
		D3D12_RESOURCE_STATES InResourceState, bool bNeedsStateTracking, RenderResourceCreateInfo& CreateInfo)
	{
		drn_check(NumElements > 0 && BytesPerElement > 0);

		drn_check((InUsage & (uint32)EBufferUsageFlags::AnyDynamic) == 0); // no support yet
		drn_check(!CreateInfo.ResourceArray); // no support yet

		uint32 Size = BytesPerElement * NumElements;
		const uint32 Alignment = BytesPerElement;

		RenderRawBuffer* Buffer = new RenderRawBuffer(InParent, 0, Size, InUsage);
		Buffer->NumElements = NumElements;
		Buffer->BytesPerElement = BytesPerElement;
		Buffer->Format = Foramt;

		RenderResource* NewResource = nullptr;
		const D3D12_RESOURCE_DESC ResourceDesc = CreateRawBufferResourceDesc( Size, InUsage );
		InParent->CreateCommittedResource(ResourceDesc, CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), InResourceState, bNeedsStateTracking, nullptr, &NewResource, CreateInfo.DebugName);
		Buffer->m_ResourceLocation.AsStandAlone(NewResource);

		if ((InUsage & (uint32)EBufferUsageFlags::ShaderResource))
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC Desc;
			Desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			Desc.Format = Buffer->Format;
			Desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			Desc.Buffer.StructureByteStride = 0;
			Desc.Buffer.FirstElement = 0;
			Desc.Buffer.NumElements = NumElements;
			Desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
			Buffer->SRV = new ShaderResourceView(InParent, Desc, Buffer->m_ResourceLocation, 0);
		}
		
		if ((InUsage & (uint32)EBufferUsageFlags::UnorderedAccess))
		{
			D3D12_UNORDERED_ACCESS_VIEW_DESC Desc;
			Desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
			Desc.Format = Buffer->Format;
			Desc.Buffer.CounterOffsetInBytes = 0;
			Desc.Buffer.FirstElement = 0;
			Desc.Buffer.StructureByteStride = 0;
			Desc.Buffer.NumElements = NumElements;
			Desc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

			Buffer->UAV = new UnorderedAccessView(InParent, Desc, Buffer->m_ResourceLocation);
		}

		return Buffer;
	}

	uint32 RenderRawBuffer::GetSrvIndex() const
	{
		return SRV ? SRV->GetDescriptorHeapIndex() : 0;
	}

	uint32 RenderRawBuffer::GetUavIndex() const
	{
		return UAV ? UAV->GetDescriptorHeapIndex() : 0;
	}

// --------------------------------------------------------------------------------------------------------

#if RENDER_STATS
	std::atomic<int32> BufferStats::BufferMemoryStats[(uint8)EBufferMemoryStatGroups::Max];
#endif

	void BufferStats::UpdateBufferStats( ResourceLocation* Location, bool bAllocating, EBufferMemoryStatGroups Group )
	{
#if RENDER_STATS
		drn_check(Group != EBufferMemoryStatGroups::Total);

		const int32 Size = bAllocating ? Location->GetSize() : -Location->GetSize();
		BufferMemoryStats[(uint8)Group].fetch_add(Size);
		BufferMemoryStats[(uint8)EBufferMemoryStatGroups::Total].fetch_add(Size);
#endif
	}

	std::string BufferStats::GetBufferStatName( EBufferMemoryStatGroups Group )
	{
		switch ( Group )
		{
		case BufferStats::EBufferMemoryStatGroups::Total:		return "TotalBuffer";
		case BufferStats::EBufferMemoryStatGroups::Index:		return "Index";
		case BufferStats::EBufferMemoryStatGroups::Vertex:		return "Vertex";
		case BufferStats::EBufferMemoryStatGroups::Constant:	return "Constant";
		case BufferStats::EBufferMemoryStatGroups::Structured:	return "Structured";
		case BufferStats::EBufferMemoryStatGroups::Buffer:		return "Buffer";
		default: drn_check(false); return "invalid";
		}
	}

	int32 BufferStats::GetBufferStatSize( EBufferMemoryStatGroups Group )
	{
#if RENDER_STATS
		return BufferMemoryStats[(uint8)Group].load();
#else
		return 0;
#endif
	}


 }  // namespace Drn