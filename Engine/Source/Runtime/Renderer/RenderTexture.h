#pragma once

#include "Runtime/Renderer/RenderTexture.h"
#include "Runtime/Renderer/RenderResource.h"
#include "Runtime/Renderer/ResourceView.h"

namespace Drn
{
	enum ETextureCreateFlags
	{
		None							= 0,
		RenderTargetable				= 1<<0,
		ResolveTargetable				= 1<<1,
		DepthStencilTargetable			= 1<<2,
		ShaderResource					= 1<<3,
		SRGB							= 1<<4,
		CPUWritable						= 1<<5,
		DisableSRVCreation				= 1<<6,
		UAV								= 1<<7,
		Presentable						= 1<<8,
		CPUReadback						= 1<<9,
		TargetArraySlicesIndependently	= 1<<10,
		NoFastClear						= 1<<11,
		DepthStencilResolveTarget		= 1<<12,
		Transient						= 1<<13
	};

	enum class EDepthStencilViewType : uint8
	{
		DepthStencilWrite	= 0,
		ReadOnlyDepth		= 1,
		ReadOnlyStencil		= 2,
		ReadOnlyDepthStencil= 3,
		Max					= 4
	};

	enum class EClearBinding
	{
		ENoneBound,
		EColorBound,
		EDepthStencilBound,
	};

	struct ClearValueBinding
	{
		struct DSVAlue
		{
			float Depth;
			uint32 Stencil;
		};

		ClearValueBinding()
			: ColorBinding(EClearBinding::EColorBound)
		{
			Value.Color[0] = 0.0f;
			Value.Color[1] = 0.0f;
			Value.Color[2] = 0.0f;
			Value.Color[3] = 0.0f;
		}

		ClearValueBinding(EClearBinding NoBinding)
			: ColorBinding(NoBinding)
		{
			drn_check(ColorBinding == EClearBinding::ENoneBound);
		}

		explicit ClearValueBinding(const Vector4& InClearColor) // TODO: Linear color
			: ColorBinding(EClearBinding::EColorBound)
		{
			Value.Color[0] = InClearColor.GetX();
			Value.Color[1] = InClearColor.GetY();
			Value.Color[2] = InClearColor.GetZ();
			Value.Color[3] = InClearColor.GetW();
		}

		explicit ClearValueBinding(float DepthClearValue, uint32 StencilClearValue = 0)
			: ColorBinding(EClearBinding::EDepthStencilBound)
		{
			Value.DSValue.Depth = DepthClearValue;
			Value.DSValue.Stencil = StencilClearValue;
		}

		Vector4 GetClearColor() const // TODO: Linear color
		{
			drn_check(ColorBinding == EClearBinding::EColorBound);
			return Vector4(Value.Color[0], Value.Color[1], Value.Color[2], Value.Color[3]);
		}

		void GetDepthStencil(float& OutDepth, uint32& OutStencil) const
		{
			drn_check(ColorBinding == EClearBinding::EDepthStencilBound);
			OutDepth = Value.DSValue.Depth;
			OutStencil = Value.DSValue.Stencil;
		}

		bool operator==(const ClearValueBinding& Other) const
		{
			if (ColorBinding == Other.ColorBinding)
			{
				if (ColorBinding == EClearBinding::EColorBound)
				{
					return
						Value.Color[0] == Other.Value.Color[0] &&
						Value.Color[1] == Other.Value.Color[1] &&
						Value.Color[2] == Other.Value.Color[2] &&
						Value.Color[3] == Other.Value.Color[3];

				}
				if (ColorBinding == EClearBinding::EDepthStencilBound)
				{
					return
						Value.DSValue.Depth == Other.Value.DSValue.Depth &&
						Value.DSValue.Stencil == Other.Value.DSValue.Stencil;
				}
				return true;
			}
			return false;
		}

		EClearBinding ColorBinding;

		union ClearValueType
		{
			float Color[4];
			DSVAlue DSValue;
		} Value;

		static const ClearValueBinding None;
		static const ClearValueBinding Black;
		static const ClearValueBinding BlackMaxAlpha;
		static const ClearValueBinding White;
		static const ClearValueBinding Transparent;
		static const ClearValueBinding DepthOne;
		static const ClearValueBinding DepthZero;
		static const ClearValueBinding DepthNear;
		static const ClearValueBinding DepthFar;	
		static const ClearValueBinding Green;
		static const ClearValueBinding DefaultNormal8Bit;
	};

	class RenderTextureBase : BaseShaderResource
	{
	public:

		RenderTextureBase(class Device* InParent)
			: BaseShaderResource(InParent)
			, MemorySize(0)
			, m_BaseShaderResource(this)
			, bCreatedRTVsPerSlice(false)
			, NumDepthStencilViews(0)
		{}

		virtual ~RenderTextureBase() {}

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
		//void InitializeTextureData(class FRHICommandListImmediate* RHICmdList, const void* InitData, uint32 InitDataSize, uint32 SizeX, uint32 SizeY, uint32 SizeZ, uint32 NumSlices, uint32 NumMips, EPixelFormat Format, D3D12_RESOURCE_STATES DestinationState);

		/**
		* Get the render target view for the specified mip and array slice.
		* An array slice of -1 is used to indicate that no array slice should be required.
		*/
		RenderTargetView* GetRenderTargetView(int32 MipIndex, int32 ArraySliceIndex) const
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
	};


	template<typename BaseResourceType>
	class TRenderTexture2D : public BaseResourceType, public RenderTextureBase
	{
	public:

		ETextureCreateFlags Flags;

		TRenderTexture2D(
			class Device* InParent,
			uint32 InSizeX,
			uint32 InSizeY,
			uint32 InSizeZ,
			uint32 InNumMips,
			uint32 InNumSamples,
			DXGI_FORMAT InFormat,
			bool bInCubemap,
			ETextureCreateFlags InFlags,
			const ClearValueBinding& InClearValue
			//const FD3D12TextureLayout* InTextureLayout = nullptr
			)
			: BaseResourceType(
				InSizeX,
				InSizeY,
				InSizeZ,
				InNumMips,
				InNumSamples,
				InFormat,
				InFlags,
				InClearValue
				)
			, RenderTextureBase(InParent)
			, Flags(InFlags)
			, bCubemap(bInCubemap)
			, bMipOrderDescending(false)
		{
			//if (InTextureLayout == nullptr)
			//{
			//	FMemory::Memzero(TextureLayout);
			//}
			//else
			//{
			//	TextureLayout = *InTextureLayout;
			//}
		}

		virtual ~TRenderTexture2D() {}

		//void* Lock(class FRHICommandListImmediate* RHICmdList, uint32 MipIndex, uint32 ArrayIndex, EResourceLockMode LockMode, uint32& DestStride);
		//void Unlock(class FRHICommandListImmediate* RHICmdList, uint32 MipIndex, uint32 ArrayIndex);
		//void UpdateTexture2D(class FRHICommandListImmediate* RHICmdList, uint32 MipIndex, const struct FUpdateTextureRegion2D& UpdateRegion, uint32 SourcePitch, const uint8* SourceData);

		RenderResource* GetResource() const { return RenderTextureBase::GetResource(); }
		//void GetReadBackHeapDesc(D3D12_PLACED_SUBRESOURCE_FOOTPRINT& OutFootprint, uint32 Subresource) const;
		bool IsCubemap() const { return bCubemap; }
		bool IsLastMipFirst() const { return bMipOrderDescending; }

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

		//const FD3D12TextureLayout& GetTextureLayout() const { return TextureLayout; }

	private:
		//void UnlockInternal(class FRHICommandListImmediate* RHICmdList, FLinkedObjectIterator NextObject, uint32 MipIndex, uint32 ArrayIndex);

		const uint32 bCubemap : 1;
		uint32 bMipOrderDescending : 1;

		//FD3D12TextureLayout TextureLayout;

		//mutable TUniquePtr<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> FirstSubresourceFootprint;
	};

	//class RenderBaseTexture2D : public FRHITexture2D
	//{
	//public:
	//	FD3D12BaseTexture2D(uint32 InSizeX, uint32 InSizeY, uint32 InSizeZ, uint32 InNumMips, uint32 InNumSamples, EPixelFormat InFormat, ETextureCreateFlags InFlags, const FClearValueBinding& InClearValue)
	//		: FRHITexture2D(InSizeX, InSizeY, InNumMips, InNumSamples, InFormat, InFlags, InClearValue)
	//	{}
	//	uint32 GetSizeZ() const { return 0; }
	//
	//	virtual void GetWriteMaskProperties(void*& OutData, uint32& OutSize) override final
	//	{
	//		FD3D12FastClearResource::GetWriteMaskProperties(OutData, OutSize);
	//	}
	//};


}