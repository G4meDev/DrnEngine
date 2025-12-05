#include "DrnPCH.h"
#include "RenderTexture.h"

namespace Drn
{
	const ClearValueBinding ClearValueBinding::None(EClearBinding::ENoneBound);
	const ClearValueBinding ClearValueBinding::Black(Vector4(0.0f, 0.0f, 0.0f, 1.0f));
	const ClearValueBinding ClearValueBinding::BlackMaxAlpha(Vector4(0.0f, 0.0f, 0.0f, FLT_MAX));
	const ClearValueBinding ClearValueBinding::White(Vector4(1.0f, 1.0f, 1.0f, 1.0f));
	const ClearValueBinding ClearValueBinding::Transparent(Vector4(0.0f, 0.0f, 0.0f, 0.0f));
	const ClearValueBinding ClearValueBinding::DepthOne(1.0f, 0);
	const ClearValueBinding ClearValueBinding::DepthZero(0.0f, 0);
	const ClearValueBinding ClearValueBinding::DepthNear(1, 0);
	const ClearValueBinding ClearValueBinding::DepthFar(0, 0);
	const ClearValueBinding ClearValueBinding::Green(Vector4(0.0f, 1.0f, 0.0f, 1.0f));
	const ClearValueBinding ClearValueBinding::DefaultNormal8Bit(Vector4(128.0f / 255.0f, 128.0f / 255.0f, 128.0f / 255.0f, 1.0f));

	RenderTexture2D* RenderTexture2D::Create(class D3D12CommandList* CmdList, uint32 SizeX, uint32 SizeY, DXGI_FORMAT Format, uint32 NumMips, uint32 NumSamples, bool bNeedsStateTracking, ETextureCreateFlags Flags, RenderResourceCreateInfo& CreateInfo)
	{
		bool bTextureArray = false;
		bool bCubeTexture = false;

		drn_check(SizeX > 0 && SizeY > 0 && NumMips > 0);
		drn_check(SizeX <= MAX_TEXTURE_SIZE_2D);

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
			1,
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

		RenderTexture2D* NewTexture = new RenderTexture2D(ParentDevice, SizeX, SizeY, NumMips, ActualMSAACount, Format, Flags, CreateInfo.ClearValueBinding);
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

						RTVDesc.Format = Format;
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

					RTVDesc.Format = Format;

					if (bTextureArray || bCubeTexture)
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
			DSVDesc.Format = Format;
			if (bTextureArray || bCubeTexture)
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
			for (uint32 AccessType = 0; AccessType < (uint8)EDepthStencilViewType::Max; ++AccessType)
			{
				if ((EDepthStencilViewType)AccessType == EDepthStencilViewType::ReadOnlyDepthStencil)
				{
					DSVDesc.Flags = D3D12_DSV_FLAG_READ_ONLY_DEPTH | D3D12_DSV_FLAG_READ_ONLY_STENCIL;
				}

				else if ((EDepthStencilViewType)AccessType == EDepthStencilViewType::ReadOnlyDepth)
				{
					DSVDesc.Flags = D3D12_DSV_FLAG_READ_ONLY_DEPTH;
				}

				else if ((EDepthStencilViewType)AccessType == EDepthStencilViewType::ReadOnlyStencil)
				{
					DSVDesc.Flags = D3D12_DSV_FLAG_READ_ONLY_STENCIL;
				}

				NewTexture->SetDepthStencilView(new DepthStencilView(ParentDevice, DSVDesc, Location, HasStencil), AccessType);
			}
		}

		if (bCreateShaderResource)
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
			SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			SRVDesc.Format = Format;

			//if (bCubeTexture && bTextureArray)
			//{
			//	SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
			//	SRVDesc.TextureCubeArray.MostDetailedMip = 0;
			//	SRVDesc.TextureCubeArray.MipLevels = NumMips;
			//	SRVDesc.TextureCubeArray.First2DArrayFace = 0;
			//	SRVDesc.TextureCubeArray.NumCubes = SizeZ / 6;
			//}
			if (false){}

			else if (bCubeTexture)
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


		//FD3D12TextureStats::D3D12TextureAllocated(*D3D12TextureOut);

		//// Initialize if data is given
		//if (CreateInfo.BulkData != nullptr)
		//{
		//	D3D12TextureOut->InitializeTextureData(RHICmdList, CreateInfo.BulkData->GetResourceBulkData(), CreateInfo.BulkData->GetResourceBulkDataSize(), SizeX, SizeY, 1, SizeZ, NumMips, Format, InitialState);
		//
		//	CreateInfo.BulkData->Discard();
		//}
		//
		//return D3D12TextureOut;

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

}