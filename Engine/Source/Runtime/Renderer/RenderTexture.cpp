#include "DrnPCH.h"
#include "RenderTexture.h"
#include <DirectXTex.h>

namespace Drn
{
	RenderTextureBase::~RenderTextureBase()
	{
		TextureStats::TextureDeleted( *this );
	}

	template<typename T>
	T* CreateTexture2D( D3D12CommandList* CmdList, uint32 SizeX, uint32 SizeY, uint32 SizeZ, uint32 NumMips, uint32 NumSamples,
		DXGI_FORMAT Format, bool bNeedsStateTracking, ETextureCreateFlags Flags, RenderResourceCreateInfo& CreateInfo, bool bTextureArray, bool bCubemap )
	{
		drn_check(SizeX > 0 && SizeY > 0 && NumMips > 0);
		drn_check(SizeX <= MAX_TEXTURE_SIZE_2D);

		// TODO: update by flag and only support typeless input
		//const bool bSRGB = (Flags & ETextureCreateFlags::SRGB) != 0;
		const bool bSRGB = IsSRGB(Format);
		DXGI_FORMAT TypelessFormat = MakeTypeless(Format);

		//const DXGI_FORMAT ShaderResourceFormat = FindShaderResourceDXGIFormat(ResourceFormat, bSRGB);
		//const DXGI_FORMAT RenderTargetFormat = FindShaderResourceDXGIFormat(ResourceFormat, bSRGB);
		//const DXGI_FORMAT DepthStencilFormat = FindDepthStencilDXGIFormat(TypelessFormat);

		bool bDepthStencil = Flags& ETextureCreateFlags::DepthStencilTargetable;

		DXGI_FORMAT ShaderResourceFormat = bDepthStencil ? FindDepthStencilSRVFormat(Format) : Format;
		DXGI_FORMAT RenderTargetFormat = Format;
		DXGI_FORMAT DepthStencilFormat = Format;

		bool bCreateShaderResource = true;

		//uint32 ActualMSAACount = NumSamples;
		//uint32 ActualMSAAQuality = GetMaxMSAAQuality(ActualMSAACount);
		//
		//// 0xffffffff means not supported
		//if (ActualMSAAQuality == 0xffffffff || (Flags & TexCreate_Shared) != 0)
		//{
		//	// no MSAA
		//	ActualMSAACount = 1;
		//	ActualMSAAQuality = 0;
		//}
		//
		//const bool bIsMultisampled = ActualMSAACount > 1;

		uint32 ActualMSAACount = 1;
		uint32 ActualMSAAQuality = 0;
		const bool bIsMultisampled = false;

		if (Flags & ETextureCreateFlags::CPUReadback)
		{
			drn_check(!(Flags & ETextureCreateFlags::RenderTargetable));
			drn_check(!(Flags & ETextureCreateFlags::DepthStencilTargetable));
			drn_check(!(Flags & ETextureCreateFlags::ShaderResource));
			bCreateShaderResource = false;
		}

		if (Flags & ETextureCreateFlags::DisableSRVCreation)
		{
			bCreateShaderResource = false;
		}

		D3D12_RESOURCE_DESC TextureDesc = CD3DX12_RESOURCE_DESC::Tex2D(
			Format,
			SizeX,
			SizeY,
			SizeZ,
			NumMips,
			ActualMSAACount,
			ActualMSAAQuality,
			D3D12_RESOURCE_FLAG_NONE);

		bool bCreateRTV = false;
		bool bCreateDSV = false;

		if (Flags & ETextureCreateFlags::RenderTargetable)
		{
			drn_check(!(Flags & ETextureCreateFlags::DepthStencilTargetable));
			drn_check(!(Flags & ETextureCreateFlags::ResolveTargetable));
			TextureDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
			bCreateRTV = true;
		}
		else if (Flags & ETextureCreateFlags::DepthStencilTargetable)
		{
			drn_check(!(Flags & ETextureCreateFlags::RenderTargetable));
			drn_check(!(Flags & ETextureCreateFlags::ResolveTargetable));
			TextureDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
			bCreateDSV = true;
		}
		//else if (Flags & TexCreate_ResolveTargetable)
		//{
		//	check(!(Flags & TexCreate_RenderTargetable));
		//	check(!(Flags & TexCreate_DepthStencilTargetable));
		//	if (Format == PF_DepthStencil || Format == PF_ShadowDepth || Format == PF_D24)
		//	{
		//		TextureDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
		//		bCreateDSV = true;
		//	}
		//	else
		//	{
		//		TextureDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		//		bCreateRTV = true;
		//	}
		//}

		if (Flags & ETextureCreateFlags::UAV)
		{
			TextureDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		}

		if (bCreateDSV && !(Flags & ETextureCreateFlags::ShaderResource))
		{
			TextureDesc.Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
			bCreateShaderResource = false;
		}

		D3D12_CLEAR_VALUE *ClearValuePtr = nullptr;
		D3D12_CLEAR_VALUE ClearValue;
		if (bCreateDSV && CreateInfo.ClearValueBinding.ColorBinding == EClearBinding::EDepthStencilBound)
		{
			ClearValue = CD3DX12_CLEAR_VALUE(Format, CreateInfo.ClearValueBinding.Value.DSValue.Depth, (uint8)CreateInfo.ClearValueBinding.Value.DSValue.Stencil);
			ClearValuePtr = &ClearValue;
		}
		else if (bCreateRTV && CreateInfo.ClearValueBinding.ColorBinding == EClearBinding::EColorBound)
		{
			ClearValue = CD3DX12_CLEAR_VALUE(Format, CreateInfo.ClearValueBinding.Value.Color);
			ClearValuePtr = &ClearValue;
		}

		// The state this resource will be in when it leaves this function
		const ResourceTypeHelper Type(TextureDesc, D3D12_HEAP_TYPE_DEFAULT);
		const D3D12_RESOURCE_STATES InitialState = Type.GetOptimalInitialState(false);

		Device* const ParentDevice = CmdList->GetParentDevice();

		T* NewTexture;
		if constexpr (std::is_same_v<T, RenderTexture2D>)
		{
			NewTexture = new RenderTexture2D(ParentDevice, SizeX, SizeY, NumMips, ActualMSAACount, Format, Flags, CreateInfo.ClearValueBinding);
		}
		else if constexpr (std::is_same_v<T, RenderTexture2DArray>)
		{
			NewTexture = new RenderTexture2DArray(ParentDevice, SizeX, SizeY, SizeZ, NumMips, ActualMSAACount, Format, Flags, CreateInfo.ClearValueBinding);
		}
		else if constexpr (std::is_same_v<T, RenderTextureCube>)
		{
			NewTexture = new RenderTextureCube(ParentDevice, SizeX, NumMips, ActualMSAACount, Format, Flags, CreateInfo.ClearValueBinding);
		}
		else
		{
			drn_check(false);
		}

		ResourceLocation& Location = NewTexture->m_ResourceLocation;

		SafeCreateTexture2D(ParentDevice, TextureDesc, ClearValuePtr, &Location, Format, Flags,
			(CreateInfo.BulkData != nullptr) ? D3D12_RESOURCE_STATE_COPY_DEST : InitialState, bNeedsStateTracking, CreateInfo.DebugName);

		uint32 RTVIndex = 0;
		if (bCreateRTV)
		{
			//const bool bCreateRTVsPerSlice = (Flags & ETextureCreateFlags::TargetArraySlicesIndependently) && (bTextureArray || bCubeTexture);
			const bool bCreateRTVsPerSlice = false;
			NewTexture->SetNumRenderTargetViews(bCreateRTVsPerSlice ? NumMips * TextureDesc.DepthOrArraySize : NumMips);

			for (uint32 MipIndex = 0; MipIndex < NumMips; MipIndex++)
			{
				if (bCreateRTVsPerSlice)
				{
					NewTexture->SetCreatedRTVsPerSlice(true, TextureDesc.DepthOrArraySize);

					for (uint32 SliceIndex = 0; SliceIndex < TextureDesc.DepthOrArraySize; SliceIndex++)
					{
						D3D12_RENDER_TARGET_VIEW_DESC RTVDesc = {};

						RTVDesc.Format = RenderTargetFormat;
						RTVDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
						RTVDesc.Texture2DArray.FirstArraySlice = SliceIndex;
						RTVDesc.Texture2DArray.ArraySize = 1;
						RTVDesc.Texture2DArray.MipSlice = MipIndex;
						RTVDesc.Texture2DArray.PlaneSlice = GetPlaneSliceFromViewFormat(Format, RTVDesc.Format);

						NewTexture->SetRenderTargetViewIndex(new RenderTargetView(ParentDevice, RTVDesc, Location), RTVIndex++);
					}
				}
				else
				{
					D3D12_RENDER_TARGET_VIEW_DESC RTVDesc = {};

					RTVDesc.Format = RenderTargetFormat;

					if (bTextureArray || bCubemap)
					{
						if (bIsMultisampled)
						{
							RTVDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY;
							RTVDesc.Texture2DMSArray.FirstArraySlice = 0;
							RTVDesc.Texture2DMSArray.ArraySize = TextureDesc.DepthOrArraySize;
						}
						else
						{
							RTVDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
							RTVDesc.Texture2DArray.FirstArraySlice = 0;
							RTVDesc.Texture2DArray.ArraySize = TextureDesc.DepthOrArraySize;
							RTVDesc.Texture2DArray.MipSlice = MipIndex;
							RTVDesc.Texture2DArray.PlaneSlice = GetPlaneSliceFromViewFormat(Format, RTVDesc.Format);
						}
					}
					else
					{
						if (bIsMultisampled)
						{
							RTVDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
							// Nothing to set
						}
						else
						{
							RTVDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
							RTVDesc.Texture2D.MipSlice = MipIndex;
							RTVDesc.Texture2D.PlaneSlice = GetPlaneSliceFromViewFormat(Format, RTVDesc.Format);
						}
					}

					NewTexture->SetRenderTargetViewIndex(new RenderTargetView(ParentDevice, RTVDesc, Location), RTVIndex++);
				}
			}
		}

		if (bCreateDSV)
		{
			D3D12_DEPTH_STENCIL_VIEW_DESC DSVDesc = {};
			DSVDesc.Format = DepthStencilFormat;
			if (bTextureArray || bCubemap)
			{
				if (bIsMultisampled)
				{
					DSVDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY;
					DSVDesc.Texture2DMSArray.FirstArraySlice = 0;
					DSVDesc.Texture2DMSArray.ArraySize = TextureDesc.DepthOrArraySize;
				}
				else
				{
					DSVDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
					DSVDesc.Texture2DArray.FirstArraySlice = 0;
					DSVDesc.Texture2DArray.ArraySize = TextureDesc.DepthOrArraySize;
					DSVDesc.Texture2DArray.MipSlice = 0;
				}
			}
			else
			{
				if (bIsMultisampled)
				{
					DSVDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
					// Nothing to set
				}
				else
				{
					DSVDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
					DSVDesc.Texture2D.MipSlice = 0;
				}
			}

			const bool HasStencil = HasStencilBits(DSVDesc.Format);
			D3D12_DSV_FLAGS StencilFlag = HasStencil ? D3D12_DSV_FLAG_READ_ONLY_STENCIL : D3D12_DSV_FLAG_NONE;

			for (uint32 AccessType = 0; AccessType < (uint8)EDepthStencilViewType::Max; ++AccessType)
			{

				if ((EDepthStencilViewType)AccessType == EDepthStencilViewType::DepthStencilRead)
				{
					DSVDesc.Flags = D3D12_DSV_FLAG_READ_ONLY_DEPTH | StencilFlag;
				}

				else if ((EDepthStencilViewType)AccessType == EDepthStencilViewType::DepthWrite)
				{
					DSVDesc.Flags = StencilFlag;
				}

				else if ((EDepthStencilViewType)AccessType == EDepthStencilViewType::StencilWrite)
				{
					DSVDesc.Flags = D3D12_DSV_FLAG_READ_ONLY_DEPTH;
				}

				NewTexture->SetDepthStencilView(new DepthStencilView(ParentDevice, DSVDesc, Location, HasStencil), AccessType);
			}
		}

		if (bCreateShaderResource)
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
			SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			SRVDesc.Format = ShaderResourceFormat;

			//if (bCubeTexture && bTextureArray)
			//{
			//	SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
			//	SRVDesc.TextureCubeArray.MostDetailedMip = 0;
			//	SRVDesc.TextureCubeArray.MipLevels = NumMips;
			//	SRVDesc.TextureCubeArray.First2DArrayFace = 0;
			//	SRVDesc.TextureCubeArray.NumCubes = SizeZ / 6;
			//}
			if (false){}

			else if (bCubemap)
			{
				SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
				SRVDesc.TextureCube.MostDetailedMip = 0;
				SRVDesc.TextureCube.MipLevels = NumMips;
			}
			else if (bTextureArray)
			{
				if (bIsMultisampled)
				{
					SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
					SRVDesc.Texture2DMSArray.FirstArraySlice = 0;
					SRVDesc.Texture2DMSArray.ArraySize = TextureDesc.DepthOrArraySize;
					//SRVDesc.Texture2DArray.PlaneSlice = GetPlaneSliceFromViewFormat(PlatformResourceFormat, SRVDesc.Format);
				}
				else
				{
					SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
					SRVDesc.Texture2DArray.MostDetailedMip = 0;
					SRVDesc.Texture2DArray.MipLevels = NumMips;
					SRVDesc.Texture2DArray.FirstArraySlice = 0;
					SRVDesc.Texture2DArray.ArraySize = TextureDesc.DepthOrArraySize;
					SRVDesc.Texture2DArray.PlaneSlice = GetPlaneSliceFromViewFormat(Format, SRVDesc.Format);
				}
			}
			else
			{
				if (bIsMultisampled)
				{
					SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
					// Nothing to set
				}
				else
				{
					SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
					SRVDesc.Texture2D.MostDetailedMip = 0;
					SRVDesc.Texture2D.MipLevels = NumMips;
					SRVDesc.Texture2D.PlaneSlice = GetPlaneSliceFromViewFormat(Format, SRVDesc.Format);
				}
			}

			NewTexture->SetShaderResourceView(new ShaderResourceView(ParentDevice, SRVDesc, Location));
		}


		TextureStats::TextureAllocated(*NewTexture);

		if (CreateInfo.BulkData != nullptr)
		{
			//NewTexture->InitializeTextureData(CmdList, CreateInfo.BulkData->GetResourceBulkData(), CreateInfo.BulkData->GetResourceBulkDataSize(), SizeX, SizeY, 1, SizeZ, NumMips, Format, InitialState);
			NewTexture->InitializeTextureData(CmdList, CreateInfo.BulkData, 0, SizeX, SizeY, 1, SizeZ, NumMips, Format, InitialState);
		
			//CreateInfo.BulkData->Discard();
		}
		
		return NewTexture;
	}

	void SafeCreateTexture2D( Device* pDevice, const D3D12_RESOURCE_DESC& TextureDesc, const D3D12_CLEAR_VALUE* ClearValue, ResourceLocation* OutTexture2D,
		DXGI_FORMAT Format, ETextureCreateFlags Flags, D3D12_RESOURCE_STATES InitialState, bool bNeedsStateTracking, const std::string& Name )
	{
		RenderResource* NewResource = nullptr;
		const D3D12_HEAP_TYPE HeapType = (Flags & ETextureCreateFlags::CPUReadback) ? D3D12_HEAP_TYPE_READBACK : D3D12_HEAP_TYPE_DEFAULT;

		switch (HeapType)
		{
		//case D3D12_HEAP_TYPE_READBACK:
		//	{
		//		uint64 Size = 0;
		//		pDevice->GetD3D12Device()->GetCopyableFootprints(&TextureDesc, 0, TextureDesc.MipLevels * TextureDesc.DepthOrArraySize, 0, nullptr, nullptr, nullptr, &Size);
		//
		//		RenderResource* Resource = nullptr;
		//		VERIFYD3D12CREATETEXTURERESULT(Adapter->CreateBuffer(HeapType, pDevice->GetGPUMask(), pDevice->GetVisibilityMask(), Size, &Resource, Name), TextureDesc, pDevice->GetDevice());
		//		OutTexture2D->AsStandAlone(Resource);
		//
		//		if (IsCPUWritable(HeapType))
		//		{
		//			OutTexture2D->SetMappedBaseAddress(Resource->Map());
		//		}
		//	}
		//	break;

		case D3D12_HEAP_TYPE_DEFAULT:
			//VERIFYD3D12CREATETEXTURERESULT(pDevice->GetTextureAllocator().AllocateTexture(TextureDesc, ClearValue, Format, *OutTexture2D, InitialState, Name), TextureDesc, pDevice->GetDevice());

			pDevice->CreateCommittedResource(TextureDesc, CD3DX12_HEAP_PROPERTIES(HeapType), InitialState, bNeedsStateTracking, ClearValue, &NewResource, Name);

			OutTexture2D->SetType(ResourceLocation::ResourceLocationType::eStandAlone);
			OutTexture2D->SetResource(NewResource);

			break;

		default:
			drn_check(false);
		}
	}

	void RenderTextureBase::InitializeTextureData( class D3D12CommandList* CmdList, const void* InitData, uint32 InitDataSize,
		uint32 SizeX, uint32 SizeY, uint32 SizeZ, uint32 NumSlices, uint32 NumMips, DXGI_FORMAT Format, D3D12_RESOURCE_STATES DestinationState )
	{
		Device* Device = GetParentDevice();
		uint32 NumSubresources = NumMips * NumSlices;

		size_t MemSize = NumSubresources * (sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) + sizeof(UINT) + sizeof(UINT64) + sizeof(D3D12_SUBRESOURCE_DATA));
		const bool bAllocateOnStack = (MemSize < 4096);
		void* Mem = bAllocateOnStack ? alloca(MemSize) : malloc(MemSize);

		D3D12_PLACED_SUBRESOURCE_FOOTPRINT* Footprints = (D3D12_PLACED_SUBRESOURCE_FOOTPRINT*) Mem;
		drn_check(Footprints);
		UINT* Rows = (UINT*) (Footprints + NumSubresources);
		drn_check(Rows);
		UINT64* RowSizeInBytes = (UINT64*) (Rows + NumSubresources);
		drn_check(RowSizeInBytes);
		D3D12_SUBRESOURCE_DATA* SubresourceData = (D3D12_SUBRESOURCE_DATA*) (RowSizeInBytes + NumSubresources);
		drn_check(RowSizeInBytes);

		uint64 Size = 0;
		const D3D12_RESOURCE_DESC& Desc = GetResource()->GetDesc();
		Device->GetD3D12Device()->GetCopyableFootprints(&Desc, 0, NumSubresources, 0, Footprints, Rows, RowSizeInBytes, &Size);

		ResourceLocation SrcResourceLoc(Device);
		uint8* DstDataBase;

		// TODO: add fast allocator
		{
			RenderResource* Resource = nullptr;
			std::string ResourceName;
#if D3D12_Debug_INFO
			static int64 ID;
			ResourceName = std::string("TextureInitalData_") + std::to_string(++ID);
#endif
		
			Device->CreateBuffer(D3D12_HEAP_TYPE_UPLOAD, Size, D3D12_RESOURCE_STATE_COPY_SOURCE, false, &Resource, ResourceName, D3D12_RESOURCE_FLAG_NONE);
			DstDataBase = (uint8*)Resource->Map();
			SrcResourceLoc.AsStandAlone(Resource, Size);
		}

		for (int32 i = 0; i < NumSubresources; i++)
		{
			SubresourceData[i].pData = (const uint8*) InitData + Footprints[i].Offset;
			SubresourceData[i].RowPitch = Footprints[i].Footprint.RowPitch;
			SubresourceData[i].SlicePitch = Footprints[i].Footprint.RowPitch * Rows[i] * Footprints[i].Footprint.Depth;
		}

		UpdateSubresources(CmdList->GetD3D12CommandList(), GetResource()->GetResource(), SrcResourceLoc.GetResource()->GetResource(), 0, NumSubresources, Size, Footprints, Rows, RowSizeInBytes, SubresourceData);

		//if (Resource->RequiresResourceStateTracking())
		//{
		//	CmdList->TransitionResourceWithTracking(Resource, DestinationState);
		//}
		//else
		//{
		//	CmdList->AddTransitionBarrier(Resource, D3D12_RESOURCE_STATE_COPY_DEST, DestinationState, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
		//}

		if (!bAllocateOnStack)
		{
			free(Mem);
		}
	}

	RenderTexture2D* RenderTexture2D::Create(D3D12CommandList* CmdList, uint32 SizeX, uint32 SizeY, DXGI_FORMAT Format, uint32 NumMips, uint32 NumSamples, bool bNeedsStateTracking, ETextureCreateFlags Flags, RenderResourceCreateInfo& CreateInfo)
	{
		return CreateTexture2D<RenderTexture2D>(CmdList, SizeX, SizeY, 1, NumMips, NumSamples, Format, bNeedsStateTracking, Flags, CreateInfo, false, false );
	}

	RenderTexture2DArray* RenderTexture2DArray::Create(D3D12CommandList* CmdList, uint32 SizeX, uint32 SizeY, uint32 ArraySize,
		DXGI_FORMAT Format, uint32 NumMips, uint32 NumSamples, bool bNeedsStateTracking, ETextureCreateFlags Flags, RenderResourceCreateInfo& CreateInfo )
	{
		return CreateTexture2D<RenderTexture2DArray>(CmdList, SizeX, SizeY, ArraySize, NumMips, NumSamples, Format, bNeedsStateTracking, Flags, CreateInfo, true, false );
	}

	RenderTextureCube* RenderTextureCube::Create( class D3D12CommandList* CmdList, uint32 SizeX, DXGI_FORMAT Format, uint32 NumMips, uint32 NumSamples, bool bNeedsStateTracking, ETextureCreateFlags Flags, RenderResourceCreateInfo& CreateInfo )
	{
		return CreateTexture2D<RenderTextureCube>(CmdList, SizeX, SizeX, 6, NumMips, NumSamples, Format, bNeedsStateTracking, Flags, CreateInfo, false, true);
	}

// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------

#if RENDER_STATS
	std::atomic<int32> TextureStats::TextureMemoryStats[(uint8)ETextureMemoryStatGroups::Max];
#endif

	bool TextureStats::ShouldCountAsTextureMemory( D3D12_RESOURCE_FLAGS MiscFlags )
	{
		return (0 == (MiscFlags & (D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL | D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS)));
	}

	TextureStats::ETextureMemoryStatGroups TextureStats::GetTextureStatEnum( D3D12_RESOURCE_FLAGS MiscFlags, bool bCubeMap, bool b3D )
	{
#if RENDER_STATS
		if (ShouldCountAsTextureMemory(MiscFlags))
		{
			if (bCubeMap)
			{
				return ETextureMemoryStatGroups::TextureCube;
			}
			else if (b3D)
			{
				return ETextureMemoryStatGroups::Texture3D;
			}
			else
			{
				return ETextureMemoryStatGroups::Texture2D;
			}
		}
		else
		{
			if (bCubeMap)
			{
				return ETextureMemoryStatGroups::RenderTargetCube;
			}
			else if (b3D)
			{
				return ETextureMemoryStatGroups::RenderTarget3D;
			}
			else
			{
				return ETextureMemoryStatGroups::RenderTarget2D;
			}
		}
#endif

		drn_check(false);
		return ETextureMemoryStatGroups::Texture2D;
	}

	std::string TextureStats::GetTextureStatName( ETextureMemoryStatGroups Group )
	{
		switch ( Group )
		{
		case TextureStats::ETextureMemoryStatGroups::TextureTotal:			return "TextureTotal";
		case TextureStats::ETextureMemoryStatGroups::RenderTargetTotal:		return "RenderTargetTotal";
		case TextureStats::ETextureMemoryStatGroups::Texture2D:				return "Texture2D";
		case TextureStats::ETextureMemoryStatGroups::Texture3D:				return "Texture3D";
		case TextureStats::ETextureMemoryStatGroups::TextureCube:			return "TextureCube";
		case TextureStats::ETextureMemoryStatGroups::RenderTarget2D:		return "RenderTarget2D";
		case TextureStats::ETextureMemoryStatGroups::RenderTarget3D:		return "RenderTarget3D";
		case TextureStats::ETextureMemoryStatGroups::RenderTargetCube:		return "RenderTargetCube";
		default: drn_check(false); return "invalid";
		}
	}

	int32 TextureStats::GetTextureStatSize( ETextureMemoryStatGroups Group )
	{
		return TextureMemoryStats[(uint8)Group].load();
	}

	void TextureStats::UpdateTextureMemoryStats( const D3D12_RESOURCE_DESC& Desc, int64 TextureSize, bool b3D, bool bCubeMap )
	{
#if RENDER_STATS
		if (TextureSize == 0) { return; }

		const int64 AlignedSize = (TextureSize > 0) ? Align(TextureSize, 1024) / 1024 : -(Align(-TextureSize, 1024) / 1024);
		TextureMemoryStats[(uint8)GetTextureStatEnum(Desc.Flags, bCubeMap, b3D)].fetch_add(AlignedSize);

		if (ShouldCountAsTextureMemory(Desc.Flags))
		{
			TextureMemoryStats[(uint8)ETextureMemoryStatGroups::TextureTotal].fetch_add(AlignedSize);
		}
		else
		{
			TextureMemoryStats[(uint8)ETextureMemoryStatGroups::RenderTargetTotal].fetch_add(AlignedSize);
		}
#endif
	}


	void TextureStats::TextureAllocated( RenderTextureBase& Texture, const D3D12_RESOURCE_DESC* Desc )
	{
		RenderResource* Resource = Texture.GetResource();
		if (Resource)
		{
			if (!Desc)
			{
				Desc = &Resource->GetDesc();
			}

			const D3D12_RESOURCE_ALLOCATION_INFO AllocationInfo = Texture.GetParentDevice()->GetD3D12Device()->GetResourceAllocationInfo(0, 1, Desc);
			const int64 TextureSize = AllocationInfo.SizeInBytes;

			Texture.SetMemorySize(TextureSize);
			UpdateTextureMemoryStats(*Desc, TextureSize, Texture.Is3D(), Texture.IsCubemap());
		}
	}

	void TextureStats::TextureDeleted( RenderTextureBase& Texture )
	{
		RenderResource* Resource = Texture.GetResource();
		if (Resource)
		{
			const D3D12_RESOURCE_DESC& Desc = Resource->GetDesc();
			const int64 TextureSize = Texture.GetMemorySize();
			drn_check(TextureSize > 0);

			UpdateTextureMemoryStats(Desc, -TextureSize, Texture.Is3D(), Texture.IsCubemap());
		}
	}


        }  // namespace Drn