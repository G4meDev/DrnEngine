#pragma once

#include "ForwardTypes.h"
#include "Runtime/Renderer/RenderResource.h"

namespace Drn
{
	class RenderBufferBase : public BaseShaderResource
	{
	public:
		RenderBufferBase (Device* InParent)
			: BaseShaderResource(InParent)
		{}
		virtual ~RenderBufferBase ()
		{}

		//void Rename(ResourceLocation& NewLocation);
		//void RenameLDAChain(ResourceLocation& NewLocation);

		void ReleaseUnderlyingResource();
	};

	class RenderIndexBuffer : public SimpleRenderResource, public RenderBufferBase
	{
	public:
		//RenderIndexBuffer()
		//	: RenderBuffer(nullptr)
		//{}

		RenderIndexBuffer(Device* InParent, uint32 InStride, uint32 InSize, uint32 InUsage)
			: SimpleRenderResource()
			, RenderBufferBase(InParent)
			, Stride(InStride)
			, Size(InSize)
			, Usage(InUsage)
		{}

		virtual ~RenderIndexBuffer();

		void ReleaseUnderlyingResource();

		static RenderIndexBuffer* Create(class Device* InParent, D3D12CommandList* CmdList, uint32 Stride, uint32 Size, uint32 InUsage, D3D12_RESOURCE_STATES InResourceState, bool bNeedsStateTracking, RenderResourceCreateInfo& CreateInfo);

		// must be 2 or 4.
		uint32 GetStride() const { return Stride; }
		uint32 GetSize() const { return Size; }
		uint32 GetUsage() const { return Usage; }

		virtual uint32 AddRef() const
		{
			return SimpleRenderResource::AddRef();
		}
		virtual uint32 Release() const
		{
			return SimpleRenderResource::Release();
		}
		virtual uint32 GetRefCount() const
		{
			return SimpleRenderResource::GetRefCount();
		}

	private:
		uint32 Stride;
		uint32 Size;
		uint32 Usage;
	};

// ---------------------------------------------------------------------------------------

	class RenderVertexBuffer : public SimpleRenderResource, public RenderBufferBase
	{
	public:
		//RenderIndexBuffer()
		//	: RenderBuffer(nullptr)
		//{}

		RenderVertexBuffer( Device* InParent, uint32 InStride, uint32 InSize, uint32 InUsage )
			: SimpleRenderResource()
			, RenderBufferBase(InParent)
			, Size(InSize)
			, Usage(InUsage)
		{}

		virtual ~RenderVertexBuffer();

		void ReleaseUnderlyingResource();

		static RenderVertexBuffer* Create(class Device* InParent, D3D12CommandList* CmdList, uint32 Size, uint32 InUsage, D3D12_RESOURCE_STATES InResourceState, bool bNeedsStateTracking, RenderResourceCreateInfo& CreateInfo);

		uint32 GetSize() const { return Size; }
		uint32 GetUsage() const { return Usage; }

		virtual uint32 AddRef() const
		{
			return SimpleRenderResource::AddRef();
		}
		virtual uint32 Release() const
		{
			return SimpleRenderResource::Release();
		}
		virtual uint32 GetRefCount() const
		{
			return SimpleRenderResource::GetRefCount();
		}

	private:
		uint32 Size;
		uint32 Usage;
	};

// ---------------------------------------------------------------------------------------

	class RenderUniformBuffer : public SimpleRenderResource
	{
	public:

		RenderUniformBuffer(class Device* InParent, uint32 InSize, EUniformBufferUsage InUniformBufferUsage)
			: SimpleRenderResource()
			, View(nullptr)
			, ResourceLocation(InParent)
			, Size(InSize)
			, UniformBufferUsage(InUniformBufferUsage)
		{}

		virtual ~RenderUniformBuffer();

		virtual uint32 AddRef() const
		{
			return SimpleRenderResource::AddRef();
		}
		virtual uint32 Release() const
		{
			return SimpleRenderResource::Release();
		}
		virtual uint32 GetRefCount() const
		{
			return SimpleRenderResource::GetRefCount();
		}

		static RenderUniformBuffer* Create(class Device* InParent, uint32 InSize, EUniformBufferUsage Usage, const void* Contents);

		uint32 GetSize() const { return Size; }
		inline class ConstantBufferView* GetShaderView() const { return View; };
		uint32 GetViewIndex() const;

	private:
		class ConstantBufferView* View;
		ResourceLocation ResourceLocation;
		const EUniformBufferUsage UniformBufferUsage;
		uint32 Size;
	};

// ---------------------------------------------------------------------------------------

	class BufferStats
	{
	public:

		enum class EBufferMemoryStatGroups : uint8
		{
			Total = 0,

			Index,
			Vertex,
			Constant,
			Structured,

			Max
		};

		static void UpdateBufferStats(ResourceLocation* Location, bool bAllocating, EBufferMemoryStatGroups Group);

		static std::string GetBufferStatName(EBufferMemoryStatGroups Group);
		static int32 GetBufferStatSize(EBufferMemoryStatGroups Group);

	private:
#if RENDER_STATS
		static std::atomic<int32> BufferMemoryStats[(uint8)EBufferMemoryStatGroups::Max];
#endif
	};
}