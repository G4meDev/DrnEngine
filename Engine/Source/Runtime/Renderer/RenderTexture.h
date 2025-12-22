#pragma once

#include "Runtime/Renderer/RenderTexture.h"
#include "Runtime/Renderer/RenderResource.h"
#include "Runtime/Renderer/ResourceView.h"

namespace Drn
{
	class RenderTextureBase : public BaseShaderResource, public SimpleRenderResource
	{
	public:

		//FRHITexture(uint32 InNumMips, uint32 InNumSamples, EPixelFormat InFormat, ETextureCreateFlags InFlags, FLastRenderTimeContainer* InLastRenderTime, const FClearValueBinding& InClearValue)
		//	: ClearValue(InClearValue)
		//	, NumMips(InNumMips)
		//	, NumSamples(InNumSamples)
		//	, Format(InFormat)
		//	, Flags(InFlags)
		//, LastRenderTime(InLastRenderTime ? *InLastRenderTime : DefaultLastRenderTime)	
		//{}

		//RenderTextureBase(class Device* InParent)
		//	: BaseShaderResource(InParent)
		//	, MemorySize(0)
		//	, m_BaseShaderResource(this)
		//	, bCreatedRTVsPerSlice(false)
		//	, NumDepthStencilViews(0)
		//{}

		RenderTextureBase(class Device* InParent, uint32 InNumMips, uint32 InNumSamples, DXGI_FORMAT InFormat, ETextureCreateFlags InFlags, const ClearValueBinding& InClearValue)
			: BaseShaderResource(InParent)
			, MemorySize(0)
			, m_BaseShaderResource(this)
			, bCreatedRTVsPerSlice(false)
			, NumDepthStencilViews(0)
			, ClearValue(InClearValue)
			, NumMips(InNumMips)
			, NumSamples(InNumSamples)
			, Format(InFormat)
			, Flags(InFlags)
		{}

		virtual ~RenderTextureBase();

		inline void SetCreatedRTVsPerSlice(bool Value, int32 InRTVArraySize)
		{ 
			bCreatedRTVsPerSlice = Value;
			RTVArraySize = InRTVArraySize;
		}

		void SetNumRenderTargetViews(int32 InNumViews)
		{
			RenderTargetViews.clear();
			RenderTargetViews.resize(InNumViews);
		}

		void SetDepthStencilView(DepthStencilView* View, uint32 SubResourceIndex)
		{
			if (SubResourceIndex < static_cast<uint8>(EDepthStencilViewType::Max))
			{
				DepthStencilViews[SubResourceIndex] = View;
				NumDepthStencilViews = std::max(SubResourceIndex + 1, NumDepthStencilViews);
			}
			else
			{
				drn_check(false);
			}
		}

		void SetRenderTargetViewIndex(RenderTargetView* View, uint32 SubResourceIndex)
		{
			if (SubResourceIndex < (uint32)RenderTargetViews.size())
			{
				RenderTargetViews[SubResourceIndex] = View;
			}
			else
			{
				drn_check(false);
			}
		}

		void SetRenderTargetView(RenderTargetView* View)
		{
			RenderTargetViews.clear();
			RenderTargetViews.push_back(View);
		}

		int64 GetMemorySize() const
		{
			return MemorySize;
		}

		void SetMemorySize(int64 InMemorySize)
		{
			drn_check(InMemorySize > 0);
			MemorySize = InMemorySize;
		}

		RenderResource* GetResource() const { return m_ResourceLocation.GetResource(); }
		ShaderResourceView* GetShaderResourceView() const { return m_ShaderResourceView; }
		BaseShaderResource* GetBaseShaderResource() const { return m_BaseShaderResource; }

		void SetShaderResourceView(ShaderResourceView* InShaderResourceView) { m_ShaderResourceView = InShaderResourceView; }

		//void UpdateTexture(const D3D12_TEXTURE_COPY_LOCATION& DestCopyLocation, uint32 DestX, uint32 DestY, uint32 DestZ, const D3D12_TEXTURE_COPY_LOCATION& SourceCopyLocation);
		//void CopyTextureRegion(uint32 DestX, uint32 DestY, uint32 DestZ, RenderTextureBase* SourceTexture, const D3D12_BOX& SourceBox);
		void InitializeTextureData(class D3D12CommandList* CmdList, const void* InitData, uint32 InitDataSize, uint32 SizeX, uint32 SizeY, uint32 SizeZ, uint32 NumSlices, uint32 NumMips, DXGI_FORMAT Format, D3D12_RESOURCE_STATES DestinationState);

		/**
		* Get the render target view for the specified mip and array slice.
		* An array slice of -1 is used to indicate that no array slice should be required.
		*/
		RenderTargetView* GetRenderTargetView(int32 MipIndex = 0 , int32 ArraySliceIndex = -1) const
		{
			int32 ArrayIndex = MipIndex;

			if (bCreatedRTVsPerSlice)
			{
				drn_check(ArraySliceIndex >= 0);
				ArrayIndex = MipIndex * RTVArraySize + ArraySliceIndex;
				drn_check(ArrayIndex < RenderTargetViews.size());
			}
			else
			{
				// Catch attempts to use a specific slice without having created the texture to support it
				drn_check(ArraySliceIndex == -1 || ArraySliceIndex == 0);
			}

			if (ArrayIndex < RenderTargetViews.size())
			{
				return RenderTargetViews[ArrayIndex];
			}
			return 0;
		}
		DepthStencilView* GetDepthStencilView(EDepthStencilViewType AccessType) const
		{
			return DepthStencilViews[static_cast<uint8>(AccessType)];
		}

		inline bool HasDepthStencilView() const
		{
			return (NumDepthStencilViews > 0);
		}

		inline bool HasRenderTargetViews() const
		{
			return (RenderTargetViews.size() > 0);
		}

		// Modifiers.
		//void SetReadBackListHandle(FD3D12CommandListHandle listToWaitFor) { ReadBackSyncPoint = listToWaitFor; }
		//FD3D12CLSyncPoint GetReadBackSyncPoint() const { return ReadBackSyncPoint; }
		//
		//FD3D12CLSyncPoint ReadBackSyncPoint;

	protected:

		int64 MemorySize;
		BaseShaderResource* m_BaseShaderResource;
		TRefCountPtr<ShaderResourceView> m_ShaderResourceView;

		std::vector<TRefCountPtr<RenderTargetView>> RenderTargetViews;
		bool bCreatedRTVsPerSlice;
		int32 RTVArraySize;

		TRefCountPtr<DepthStencilView> DepthStencilViews[static_cast<uint8>(EDepthStencilViewType::Max)];
		uint32 NumDepthStencilViews;

		//TMap<uint32, FD3D12LockedResource*> LockedMap;

// ---------------------------------------------------------------------------------------------------

	public:
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

		virtual bool IsCubemap() const	{ return false; }
		virtual bool Is3D() const		{ return false; }

	public:
	
		virtual IntVector GetSizeXYZ() const = 0;

		uint32 GetNumMips() const { return NumMips; }
		DXGI_FORMAT GetFormat() const { return Format; }

		ETextureCreateFlags GetFlags() const { return Flags; }
		uint32 GetNumSamples() const { return NumSamples; }
		bool IsMultisampled() const { return NumSamples > 1; }

		void SetName(const std::string& InName) { TextureName = InName; }
		std::string GetName() const { return TextureName; }

		bool HasClearValue() const
		{
			return ClearValue.ColorBinding != EClearBinding::ENoneBound;
		}

		Vector4 GetClearColor() const
		{
			return ClearValue.GetClearColor();
		}

		void GetDepthStencilClearValue(float& OutDepth, uint32& OutStencil) const
		{
			return ClearValue.GetDepthStencil(OutDepth, OutStencil);
		}

		float GetDepthClearValue() const
		{
			float Depth;
			uint32 Stencil;
			ClearValue.GetDepthStencil(Depth, Stencil);
			return Depth;
		}

		uint32 GetStencilClearValue() const
		{
			float Depth;
			uint32 Stencil;
			ClearValue.GetDepthStencil(Depth, Stencil);
			return Stencil;
		}

		const ClearValueBinding GetClearBinding() const
		{
			return ClearValue;
		}

		virtual void GetWriteMaskProperties(void*& OutData, uint32& OutSize)
		{
			OutData = nullptr;
			OutSize = 0;
		}

	private:
		ClearValueBinding ClearValue;
		uint32 NumMips;
		uint32 NumSamples;
		DXGI_FORMAT Format;
		ETextureCreateFlags Flags;
		std::string TextureName;
	};

	class RenderBaseTexture2D : public RenderTextureBase
	{
	public:
	
		RenderBaseTexture2D(Device* InParent, uint32 InSizeX,uint32 InSizeY,uint32 InSizeZ,uint32 InNumMips,uint32 InNumSamples, DXGI_FORMAT InFormat,ETextureCreateFlags InFlags, const ClearValueBinding& InClearValue)
			: RenderTextureBase(InParent, InNumMips, InNumSamples, InFormat, InFlags, InClearValue)
			, SizeX(InSizeX)
			, SizeY(InSizeY)
			, SizeZ(InSizeZ)
		{}

		virtual ~RenderBaseTexture2D() {}

		uint32 GetSizeX() const { return SizeX; }
		uint32 GetSizeY() const { return SizeY; }
		uint32 GetSizeZ() const { return SizeZ; }
		inline IntPoint GetSizeXY() const { return IntPoint(SizeX, SizeY); }
		virtual IntVector GetSizeXYZ() const override { return IntVector(SizeX, SizeY, SizeZ); }

	private:

		uint32 SizeX;
		uint32 SizeY;
		uint32 SizeZ;

// -------------------------------------------------------------------------------------------------------------------------------------

	public:

		//void* Lock(class FRHICommandListImmediate* RHICmdList, uint32 MipIndex, uint32 ArrayIndex, EResourceLockMode LockMode, uint32& DestStride);
		//void Unlock(class FRHICommandListImmediate* RHICmdList, uint32 MipIndex, uint32 ArrayIndex);
		//void UpdateTexture2D(class FRHICommandListImmediate* RHICmdList, uint32 MipIndex, const struct FUpdateTextureRegion2D& UpdateRegion, uint32 SourcePitch, const uint8* SourceData);

		//void GetReadBackHeapDesc(D3D12_PLACED_SUBRESOURCE_FOOTPRINT& OutFootprint, uint32 Subresource) const;
		//bool IsCubemap() const { return bCubemap; }


	private:
		//void UnlockInternal(class FRHICommandListImmediate* RHICmdList, FLinkedObjectIterator NextObject, uint32 MipIndex, uint32 ArrayIndex);

		//const uint32 bCubemap : 1;

		//mutable TUniquePtr<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> FirstSubresourceFootprint;
	};

	class RenderTexture2D : public RenderBaseTexture2D
	{
	public:
		RenderTexture2D(Device* InParent, uint32 InSizeX, uint32 InSizeY, uint32 InNumMips, uint32 InNumSamples, DXGI_FORMAT InFormat, ETextureCreateFlags InFlags, const ClearValueBinding& InClearValue)
			: RenderBaseTexture2D(InParent, InSizeX, InSizeY, 1, InNumMips, InNumSamples, InFormat, InFlags, InClearValue)
		{
			
		}

		virtual ~RenderTexture2D() {}

		static RenderTexture2D* Create(class D3D12CommandList* CmdList, uint32 SizeX, uint32 SizeY, DXGI_FORMAT Format, uint32 NumMips, uint32 NumSamples, bool bNeedsStateTracking, ETextureCreateFlags Flags, RenderResourceCreateInfo& CreateInfo);
	};

	class RenderTexture2DArray : public RenderBaseTexture2D
	{
	public:
		RenderTexture2DArray(Device* InParent, uint32 InSizeX, uint32 InSizeY, uint32 InSizeZ, uint32 InNumMips, uint32 InNumSamples, DXGI_FORMAT InFormat, ETextureCreateFlags InFlags, const ClearValueBinding& InClearValue)
			: RenderBaseTexture2D(InParent, InSizeX, InSizeY, InSizeZ, InNumMips, InNumSamples, InFormat, InFlags, InClearValue)
		{
		}

		virtual ~RenderTexture2DArray() {}

		static RenderTexture2DArray* Create(class D3D12CommandList* CmdList, uint32 SizeX, uint32 SizeY, uint32 ArraySize, DXGI_FORMAT Format, uint32 NumMips, uint32 NumSamples, bool bNeedsStateTracking, ETextureCreateFlags Flags, RenderResourceCreateInfo& CreateInfo);
	};

	class RenderTextureCube : public RenderBaseTexture2D
	{
	public:
		RenderTextureCube(Device* InParent,uint32 InSizeX, uint32 InNumMips, uint32 InNumSamples, DXGI_FORMAT InFormat, ETextureCreateFlags InFlags, const ClearValueBinding& InClearValue)
			: RenderBaseTexture2D(InParent, InSizeX, InSizeX, 6, InNumSamples, InNumMips, InFormat, InFlags, InClearValue)
		{
		}

		virtual ~RenderTextureCube() {}

		static RenderTextureCube* Create(class D3D12CommandList* CmdList, uint32 SizeX, DXGI_FORMAT Format, uint32 NumMips, uint32 NumSamples, bool bNeedsStateTracking, ETextureCreateFlags Flags, RenderResourceCreateInfo& CreateInfo);

		virtual bool IsCubemap() const override { return true; }

	private:

	};

	template<typename T>
	T* CreateTexture2D(class D3D12CommandList* CmdList, uint32 SizeX, uint32 SizeY, uint32 SizeZ, uint32 NumMips, uint32 NumSamples, DXGI_FORMAT Format, bool bNeedsStateTracking, ETextureCreateFlags Flags, RenderResourceCreateInfo& CreateInfo, bool bTextureArray, bool bCubemap);

	void SafeCreateTexture2D( Device* pDevice, const D3D12_RESOURCE_DESC& TextureDesc, const D3D12_CLEAR_VALUE* ClearValue, ResourceLocation* OutTexture2D,
		DXGI_FORMAT Format, ETextureCreateFlags Flags, D3D12_RESOURCE_STATES InitialState, bool bNeedsStateTracking, const std::string& Name);


	class TextureStats
	{
	public:

		enum class ETextureMemoryStatGroups : uint8
		{
			TextureTotal = 0,
			RenderTargetTotal,

			Texture2D,
			Texture3D,
			TextureCube,
			RenderTarget2D,
			RenderTarget3D,
			RenderTargetCube,

			Max
		};

		static bool ShouldCountAsTextureMemory(D3D12_RESOURCE_FLAGS MiscFlags);
		static ETextureMemoryStatGroups GetTextureStatEnum(D3D12_RESOURCE_FLAGS MiscFlags, bool bCubeMap, bool b3D);
		static void UpdateTextureMemoryStats(const D3D12_RESOURCE_DESC& Desc, int64 TextureSize, bool b3D, bool bCubeMap);

		static void TextureAllocated(RenderTextureBase& Texture, const D3D12_RESOURCE_DESC *Desc = nullptr);
		static void TextureDeleted(RenderTextureBase& Texture);

		static std::string GetTextureStatName(ETextureMemoryStatGroups Group);
		static int32 GetTextureStatSize(ETextureMemoryStatGroups Group);

	private:
#if RENDER_STATS
		static std::atomic<int32> TextureMemoryStats[(uint8)ETextureMemoryStatGroups::Max];
#endif
	};
}