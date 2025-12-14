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
		default: drn_check(false); return "invalid";
		}
	}

	int32 BufferStats::GetBufferStatSize( EBufferMemoryStatGroups Group )
	{
		return BufferMemoryStats[(uint8)Group].load();
	}



        }  // namespace Drn