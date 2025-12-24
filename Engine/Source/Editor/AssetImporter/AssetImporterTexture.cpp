#include "DrnPCH.h"
#include "AssetImporterTexture.h"

#if WITH_EDITOR

#include "Runtime/Renderer/CommonResources.h"
#include "Runtime/Renderer/Texture/TextureHelper.h"
#include <renderdoc_app.h>

#include <d3d11.h>
//#include <dxgi1_2.h>

LOG_DEFINE_CATEGORY( LogAssetImporterTexture2D, "AssetImporterTexture2D" );

namespace Drn
{
	void AssetImporterTexture::Import( Texture2D* TextureAsset, const std::string& Path )
	{
		std::string Extension = Path::GetFileExtension(Path);

		if ( Extension == ".dds" || Extension == ".DDS" )
		{
			ImportDDS(TextureAsset, Path);
			return;
		}

		TexMetadata  metadata;
		ScratchImage scratchImage;
		HRESULT Result = E_FAIL;
		if ( Extension == ".tga" )
		{
			Result = LoadFromTGAFile(StringHelper::s2ws(Path).c_str(), &metadata, scratchImage);
		}

		if (Result == S_OK && metadata.dimension == TEX_DIMENSION_TEXTURE2D)
		{
			TextureAsset->m_SizeX = metadata.width;
			TextureAsset->m_SizeY = metadata.height;
			TextureAsset->m_Format = metadata.format;
			TextureAsset->m_MipLevels = 1;

			const DirectX::Image* BaseImage = scratchImage.GetImage(0, 0, 0);

			ScratchImage MipImage;
			if (TextureAsset->m_GenerateMips)
			{
				uint16 SizeMin = std::min(metadata.width, metadata.height);
				uint16 LowerPow2 = Math::RoundDownToPowerOfTwo(SizeMin);
				TextureAsset->m_MipLevels = std::log2(LowerPow2) + 1;
			
				CoInitializeEx( nullptr, COINITBASE_MULTITHREADED );
				GenerateMipMaps( *BaseImage, TEX_FILTER_CUBIC | TEX_FILTER_SEPARATE_ALPHA, TextureAsset->m_MipLevels, MipImage);
			}

			TextureAsset->m_Format = TextureHelper::ResolveTextureFormat(metadata.format, TextureAsset->GetCompression());

			const ScratchImage& RawImage = TextureAsset->m_GenerateMips ? MipImage : scratchImage;
			ScratchImage CompressedImage;
			const bool HasCompression = TextureAsset->GetCompression() != ETextureCompression::NoCompression;
			if (HasCompression)
			{
				CoInitializeEx( nullptr, COINITBASE_MULTITHREADED );

				// TODO: log errors
				HRESULT Res = Compress(RawImage.GetImages(), RawImage.GetImageCount(), RawImage.GetMetadata(),
					TextureAsset->GetFormat(), TEX_COMPRESS_DEFAULT, TEX_THRESHOLD_DEFAULT, CompressedImage);
			}

			const ScratchImage& TargetImage = HasCompression ? CompressedImage : (TextureAsset->m_GenerateMips ? MipImage : scratchImage);

			uint32 SubresourceCount = TextureAsset->GetMipLevels();

			uint64 TextureMemorySize = 0;
			std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> Layouts;
			Layouts.resize(SubresourceCount);

			std::vector<uint32> NumRows;
			NumRows.resize(SubresourceCount);

			std::vector<uint64> RowSizeInBytes;
			RowSizeInBytes.resize(SubresourceCount);

			D3D12_RESOURCE_DESC Desc = CD3DX12_RESOURCE_DESC::Tex2D(TextureAsset->GetFormat(), metadata.width, metadata.height, 1, SubresourceCount);
			Renderer::Get()->GetD3D12Device()->GetCopyableFootprints(&Desc, 0, SubresourceCount, 0, Layouts.data(), NumRows.data(), RowSizeInBytes.data(), &TextureMemorySize );

			TextureAsset->ReleaseImageBlobs();
			D3DCreateBlob(TextureMemorySize, &TextureAsset->m_ImageBlob);

			if (TextureAsset->IsSRGB())
			{
				TextureAsset->m_Format = MakeSRGB(TextureAsset->GetFormat());
			}

			for (int32 i = 0; i < SubresourceCount; i++)
			{
				const DirectX::Image* MipSource = TargetImage.GetImage( i, 0, 0 );

				const uint64 RowSize = RowSizeInBytes[i];
				for ( uint64 j = 0; j < NumRows[i]; j++ )
				{
					BYTE* CopyDest = (BYTE*)TextureAsset->m_ImageBlob->GetBufferPointer() + Layouts[i].Offset + Layouts[i].Footprint.RowPitch * j;
					BYTE* CopySource = (BYTE*)MipSource->pixels + MipSource->rowPitch * j;
					const uint64 CopySize   = MipSource->rowPitch;

					memcpy( CopyDest, CopySource, CopySize );
				}
			}
		}

		else
		{
			LOG(LogAssetImporterTexture2D, Error, "faield to load texture2d from %s", Path.c_str());
		}
	}

	void AssetImporterTexture::ImportDDS( Texture2D* TextureAsset, const std::string& Path )
	{
		TexMetadata  metadata;
		ScratchImage scratchImage;
		HRESULT Result = E_FAIL;
		Result = LoadFromDDSFile(StringHelper::s2ws(Path).c_str(), DDS_FLAGS_NONE, &metadata, scratchImage);

		if (Result == S_OK && metadata.dimension == TEX_DIMENSION_TEXTURE2D)
		{
			const uint32 MipCount = metadata.mipLevels;
			const uint32 SubresourceCount = MipCount;

			uint64 TextureMemorySize = 0;
			std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> Layouts;
			Layouts.resize(SubresourceCount);

			std::vector<uint32> NumRows;
			NumRows.resize(SubresourceCount);

			std::vector<uint64> RowSizeInBytes;
			RowSizeInBytes.resize(SubresourceCount);

			D3D12_RESOURCE_DESC Desc = CD3DX12_RESOURCE_DESC::Tex2D(TextureAsset->GetFormat(), metadata.width, metadata.height, 1, SubresourceCount);
			Renderer::Get()->GetD3D12Device()->GetCopyableFootprints(&Desc, 0, SubresourceCount, 0, Layouts.data(), NumRows.data(), RowSizeInBytes.data(), &TextureMemorySize );

			ScratchImage& TargetImage = scratchImage;

			TextureAsset->ReleaseImageBlobs();
			D3DCreateBlob(TextureMemorySize, &TextureAsset->m_ImageBlob);

			for (int32 i = 0; i < SubresourceCount; i++)
			{
				const DirectX::Image* MipSource = TargetImage.GetImage( i, 0, 0 );

				const uint64 RowSize = RowSizeInBytes[i];
				for ( uint64 j = 0; j < NumRows[i]; j++ )
				{
					BYTE* CopyDest = (BYTE*)TextureAsset->m_ImageBlob->GetBufferPointer() + Layouts[i].Offset + Layouts[i].Footprint.RowPitch * j;
					BYTE* CopySource = (BYTE*)MipSource->pixels + MipSource->rowPitch * j;
					const uint64 CopySize   = MipSource->rowPitch;

					memcpy( CopyDest, CopySource, CopySize );
				}
			}

			const DirectX::Image* BaseImage = scratchImage.GetImage(0, 0, 0);
			TextureAsset->m_SizeX = metadata.width;
			TextureAsset->m_SizeY = metadata.height;
			TextureAsset->m_MipLevels = MipCount;
			TextureAsset->m_Format = TextureAsset->IsSRGB() ? MakeSRGB(metadata.format) : metadata.format;
		}

	}

	void AssetImporterTexture::Import( TextureCube* TextureAsset, const std::string& Path )
	{
		std::string Extension = Path::GetFileExtension(Path);

		TexMetadata  metadata;
		ScratchImage scratchImage;
		HRESULT Result = E_FAIL;

		if ( Extension == ".HDR" || Extension == ".hdr" )
		{
			Result = LoadFromHDRFile(StringHelper::s2ws(Path).c_str(), &metadata, scratchImage);
		}

		if (Result == S_OK && metadata.dimension == TEX_DIMENSION_TEXTURE2D)
		{
			ImportTextureCubeFromTexture2D(TextureAsset, metadata, scratchImage);
		}

		else
		{
			LOG(LogAssetImporterTexture2D, Error, "faield to load texture cube from %s", Path.c_str());
		}
	}

	void AssetImporterTexture::ImportTextureCubeFromTexture2D( TextureCube* TextureAsset, TexMetadata& MetaData, ScratchImage& Image )
	{
		const DirectX::Image* BaseImage = Image.GetImage(0, 0, 0);
		const uint32 ImageBufferSize = BaseImage->slicePitch;

		Microsoft::WRL::ComPtr<ID3DBlob> ImageBlob;
		D3DCreateBlob(ImageBufferSize, ImageBlob.GetAddressOf());
		memcpy(ImageBlob->GetBufferPointer(), BaseImage->pixels, ImageBufferSize);

// -----------------------------------------------------------------------------------------------------------------------------------------

		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		TRefCountPtr<D3D12CommandList> CommandList = new D3D12CommandList(Renderer::Get()->GetDevice(), D3D12_COMMAND_LIST_TYPE_DIRECT, 1, "Texture2DToCubemap");
		CommandList->Close();
		CommandList->FlipAndReset();

// -----------------------------------------------------------------------------------------------------------------------------------------

		D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = 
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
			D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED |
			D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED;

		std::vector<D3D12_ROOT_PARAMETER> rootParameters;

		D3D12_ROOT_PARAMETER ViewBufferParam = {};
		ViewBufferParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		ViewBufferParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		ViewBufferParam.Constants.Num32BitValues = 32;
		ViewBufferParam.Constants.ShaderRegister = 0;
		ViewBufferParam.Constants.RegisterSpace = 0;

		rootParameters.push_back(ViewBufferParam);

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription(rootParameters.size(), rootParameters.data(), 0, nullptr, rootSignatureFlags );
		
		ID3DBlob* pSerializedRootSig;
		ID3DBlob* pRootSigError;
		HRESULT Result = D3D12SerializeVersionedRootSignature(&rootSignatureDescription, &pSerializedRootSig, &pRootSigError);
		if ( FAILED(Result) )
		{
			if ( pRootSigError )
			{
				LOG(LogMaterial, Error, "shader signature serialization failed. %s", (char*)pRootSigError->GetBufferPointer());
				pRootSigError->Release();
			}
		
			if (pSerializedRootSig)
			{
				pSerializedRootSig->Release();
				pSerializedRootSig = nullptr;
			}
		
			return;
		}
		
		Microsoft::WRL::ComPtr<ID3D12RootSignature> IntermediateRootSinature;
		Renderer::Get()->GetD3D12Device()->CreateRootSignature(NULL, pSerializedRootSig->GetBufferPointer(),
			pSerializedRootSig->GetBufferSize(), IID_PPV_ARGS(IntermediateRootSinature.GetAddressOf()));

#if D3D12_Debug_INFO
		IntermediateRootSinature->SetName(L"Texture2DToCubemapRootSignature");
#endif


// -----------------------------------------------------------------------------------------------------------------------------------------

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> SrvHeap;
		{
			D3D12_DESCRIPTOR_HEAP_DESC desc = {};
			desc.Type                       = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			desc.NumDescriptors             = 256;
			desc.Flags                      = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			Renderer::Get()->GetD3D12Device()->CreateDescriptorHeap( &desc, IID_PPV_ARGS( SrvHeap.GetAddressOf() ) );

#if D3D12_Debug_INFO
			SrvHeap->SetName(L"Texture2DToCubemapSrvHeap");
#endif
		}

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> SamplerHeap;
		{
			D3D12_DESCRIPTOR_HEAP_DESC desc = {};
			desc.Type                       = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
			desc.NumDescriptors             = 1;
			desc.Flags                      = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			Renderer::Get()->GetD3D12Device()->CreateDescriptorHeap( &desc, IID_PPV_ARGS( SamplerHeap.GetAddressOf() ) );

#if D3D12_Debug_INFO
			SamplerHeap->SetName(L"Texture2DToCubemapSamplerHeap");
#endif
		}

		ID3D12DescriptorHeap* const Heaps[2] = { SrvHeap.Get(), SamplerHeap.Get() };
		CommandList->GetD3D12CommandList()->SetDescriptorHeaps(2, Heaps);
		CommandList->GetD3D12CommandList()->SetGraphicsRootSignature(IntermediateRootSinature.Get());

		D3D12_CPU_DESCRIPTOR_HANDLE LinearSamplerCpuHandle = SamplerHeap->GetCPUDescriptorHandleForHeapStart();
		D3D12_GPU_DESCRIPTOR_HANDLE LinearSamplerGpuHandle = SamplerHeap->GetGPUDescriptorHandleForHeapStart();
		D3D12_SAMPLER_DESC SamplerDesc = {};
		SamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		SamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		SamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		SamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		Device->CreateSampler(&SamplerDesc, LinearSamplerCpuHandle);

		uint32 SrvHeapStackPointer = 0;
		auto AllocateSrv = [&]() { return SrvHeapStackPointer++; };

		auto SrvCpuHandle = [&]( uint32 Index ) { return CD3DX12_CPU_DESCRIPTOR_HANDLE(SrvHeap->GetCPUDescriptorHandleForHeapStart(), Index, Renderer::Get()->GetSrvIncrementSize()); };
		auto SrvGpuHandle = [&]( uint32 Index ) { return CD3DX12_GPU_DESCRIPTOR_HANDLE(SrvHeap->GetGPUDescriptorHandleForHeapStart(), Index, Renderer::Get()->GetSrvIncrementSize()); };

// -----------------------------------------------------------------------------------------------------------------------------------------

		D3D12_RESOURCE_DESC TextureDesc = {};
		TextureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		TextureDesc.Width = MetaData.width;
		TextureDesc.Height = MetaData.height;
		TextureDesc.DepthOrArraySize = 1;
		TextureDesc.MipLevels = 1;
		TextureDesc.Format = MetaData.format;
		TextureDesc.SampleDesc.Count = 1;
		TextureDesc.SampleDesc.Quality = 0;
		TextureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		TextureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		CD3DX12_HEAP_PROPERTIES HeapProp(D3D12_HEAP_TYPE_DEFAULT);
		Microsoft::WRL::ComPtr<ID3D12Resource> IntermediateTexture2D;
		Device->CreateCommittedResource( &HeapProp,
			D3D12_HEAP_FLAG_NONE, &TextureDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(IntermediateTexture2D.GetAddressOf()));

		D3D12_SUBRESOURCE_DATA TextureResource;
		TextureResource.pData = ImageBlob->GetBufferPointer();
		TextureResource.RowPitch = BaseImage->rowPitch;
		TextureResource.SlicePitch = BaseImage->slicePitch;

		uint64 UploadBufferSize = GetRequiredIntermediateSize(IntermediateTexture2D.Get(), 0, 1);

		HeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		D3D12_RESOURCE_DESC IntermediateDesc = CD3DX12_RESOURCE_DESC::Buffer( UploadBufferSize );
		Microsoft::WRL::ComPtr<ID3D12Resource> IntermediateResource;
		Device->CreateCommittedResource( &HeapProp,
			D3D12_HEAP_FLAG_NONE, &IntermediateDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(IntermediateResource.GetAddressOf()));

#if D3D12_Debug_INFO
		IntermediateTexture2D->SetName(L"Texture2DToCubemapResource");
		IntermediateResource->SetName(L"Texture2DToCubemapIntermediateResource");
#endif

		UpdateSubresources(CommandList->GetD3D12CommandList(), IntermediateTexture2D.Get(), IntermediateResource.Get(), 0, 0, 1, &TextureResource);

		{
			CD3DX12_RESOURCE_BARRIER Barrier = CD3DX12_RESOURCE_BARRIER::Transition(IntermediateTexture2D.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
			CommandList->GetD3D12CommandList()->ResourceBarrier(1, &Barrier);
		}

		D3D12_SHADER_RESOURCE_VIEW_DESC ResourceViewDesc = {};
		ResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		ResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		ResourceViewDesc.Format = MetaData.format;
		ResourceViewDesc.Texture2D.MipLevels = 1;
		ResourceViewDesc.Texture2D.MostDetailedMip = 0;

		uint32 IntermediateTexture2DSrvIndex = AllocateSrv();
		Device->CreateShaderResourceView(IntermediateTexture2D.Get(), &ResourceViewDesc, SrvCpuHandle(IntermediateTexture2DSrvIndex));

// -----------------------------------------------------------------------------------------------------------------------------------------

		uint64 TexelCount = BaseImage->width * BaseImage->height;
		uint32 FaceSize = Math::RoundUpToPowerOfTwo( std::sqrt(TexelCount / 6.0f) );

		uint32 MipCount = 1;
		if (TextureAsset->m_GenerateMips)
		{
			MipCount = std::log2(FaceSize) + 1;
		}

		HeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		D3D12_RESOURCE_DESC TargetDesc = CD3DX12_RESOURCE_DESC::Tex2D(MetaData.format, FaceSize, FaceSize, 6, MipCount, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);
		Microsoft::WRL::ComPtr<ID3D12Resource> TextureCubeTarget;
		Device->CreateCommittedResource( &HeapProp, D3D12_HEAP_FLAG_NONE, &TargetDesc, D3D12_RESOURCE_STATE_RENDER_TARGET, nullptr, IID_PPV_ARGS(TextureCubeTarget.GetAddressOf()));

#if D3D12_Debug_INFO
		TextureCubeTarget->SetName(L"Texture2DToCubeTarget");
#endif

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> RtvHeap;

		D3D12_DESCRIPTOR_HEAP_DESC HeapDesc = {};
		HeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		HeapDesc.NumDescriptors = MipCount;
		Device->CreateDescriptorHeap( &HeapDesc, IID_PPV_ARGS(RtvHeap.GetAddressOf()) );
#if D3D12_Debug_INFO
		RtvHeap->SetName(L"RtvHeapTexture2DToCubeMap");
#endif

		std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> TargetHandles;
		TargetHandles.resize(MipCount);

		for (int32 i = 0; i < MipCount; i++)
		{
			TargetHandles[i] = CD3DX12_CPU_DESCRIPTOR_HANDLE( RtvHeap->GetCPUDescriptorHandleForHeapStart(), i, Renderer::Get()->GetRtvIncrementSize() );

			D3D12_RENDER_TARGET_VIEW_DESC ViewDesc = {};
			ViewDesc.Format = MetaData.format;
			ViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
			ViewDesc.Texture2DArray.MipSlice = i;
			ViewDesc.Texture2DArray.ArraySize = 6;
			ViewDesc.Texture2DArray.FirstArraySlice = 0;
			Device->CreateRenderTargetView( TextureCubeTarget.Get(), &ViewDesc, TargetHandles[i] );
		}

		std::vector<uint32> TargetSrvs;
		TargetSrvs.resize(MipCount);
		for (int32 i = 0; i < MipCount; i++)
		{
			TargetSrvs[i] = AllocateSrv();
			D3D12_SHADER_RESOURCE_VIEW_DESC ViewDesc = {};
			ViewDesc.Format = MetaData.format;
			ViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			ViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
			ViewDesc.TextureCube.MipLevels = 1;
			ViewDesc.TextureCube.MostDetailedMip = i;
			Device->CreateShaderResourceView(TextureCubeTarget.Get(), &ViewDesc, SrvCpuHandle(TargetSrvs[i]));
		}

// -----------------------------------------------------------------------------------------------------------------------------------------

		struct SliceData
		{
			Matrix WorldToProjectionMatrices[6];

			uint32 TextureIndex;
			uint32 LinearSamplerIndex;
			uint32 Pad_1;
			uint32 Pad_2;
		};

		auto CalculateWorldToProjectionForDirection = [&]( Matrix& Mat, const Vector& Direction, const Vector& UpVector)
		{
			XMVECTOR LightPosition = XMLoadFloat3(Vector::ZeroVector.Get());
			XMVECTOR ViewDirection = XMLoadFloat3(Direction.Get());
			XMVECTOR FocusPoint = LightPosition + ViewDirection;

			Matrix ViewMatrix = XMMatrixLookAtLH( LightPosition, FocusPoint, XMLoadFloat3(UpVector.Get()));
			Matrix ProjectionMatrix = XMMatrixPerspectiveFovLH( XM_PIDIV2, 1.0f, 0.001, 1);
			Mat = ViewMatrix * ProjectionMatrix;
		};

		SliceData BufferData;
		CalculateWorldToProjectionForDirection(BufferData.WorldToProjectionMatrices[0], Vector::RightVector, Vector::UpVector);
		CalculateWorldToProjectionForDirection(BufferData.WorldToProjectionMatrices[1], Vector::LeftVector, Vector::UpVector);
		CalculateWorldToProjectionForDirection(BufferData.WorldToProjectionMatrices[2], Vector::UpVector, Vector::BackwardVector);
		CalculateWorldToProjectionForDirection(BufferData.WorldToProjectionMatrices[3], Vector::DownVector, Vector::ForwardVector);
		CalculateWorldToProjectionForDirection(BufferData.WorldToProjectionMatrices[4], Vector::ForwardVector, Vector::UpVector);
		CalculateWorldToProjectionForDirection(BufferData.WorldToProjectionMatrices[5], Vector::BackwardVector, Vector::UpVector);

		BufferData.TextureIndex = IntermediateTexture2DSrvIndex;
		BufferData.LinearSamplerIndex = 0; // linear sampler

		HeapProp = CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_UPLOAD );
		D3D12_RESOURCE_DESC SliceBufferDesc = CD3DX12_RESOURCE_DESC::Buffer( 512 );
		Microsoft::WRL::ComPtr<ID3D12Resource> SliceBuffer;
		Device->CreateCommittedResource(&HeapProp, D3D12_HEAP_FLAG_NONE, &SliceBufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(SliceBuffer.GetAddressOf()) );
#if D3D12_Debug_INFO
		SliceBuffer->SetName(L"CB_Texture2DToCubeMapSlice");
#endif
		uint32 SliceBufferSrvIndex = AllocateSrv();

		D3D12_CONSTANT_BUFFER_VIEW_DESC BufferDesc = {};
		BufferDesc.BufferLocation = SliceBuffer->GetGPUVirtualAddress();
		BufferDesc.SizeInBytes = 512;
		Renderer::Get()->GetD3D12Device()->CreateConstantBufferView(&BufferDesc, SrvCpuHandle(SliceBufferSrvIndex));

		{
			UINT8* ConstantBufferStart;
			CD3DX12_RANGE readRange( 0, 0 );
			SliceBuffer->Map(0, &readRange, reinterpret_cast<void**>( &ConstantBufferStart ) );
			memcpy( ConstantBufferStart, &BufferData, sizeof(SliceData));
			SliceBuffer->Unmap(0, nullptr);
		}

		CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, SliceBufferSrvIndex, 0);

// -----------------------------------------------------------------------------------------------------------------------------------------

		TRefCountPtr<Texture2DToTextureCubePSO> SlicePso = new Texture2DToTextureCubePSO(CommandList, IntermediateRootSinature.Get(), MetaData.format, CommonResources::Get());

// -----------------------------------------------------------------------------------------------------------------------------------------

		CommandList->SetViewport(0, 0, 0, FaceSize, FaceSize, 1);
		CommandList->GetD3D12CommandList()->OMSetRenderTargets(1, &TargetHandles[0], true, NULL);

		CommandList->SetGraphicPipelineState(SlicePso->m_PSO);
		CommonResources::Get()->m_UniformCube->BindAndDraw(CommandList);

// -----------------------------------------------------------------------------------------------------------------------------------------

		std::vector<SliceData> MipBufferData;
		MipBufferData.resize(MipCount - 1);

		std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> MipBuffers;
		MipBuffers.resize(MipCount - 1);

		for (int32 i = 1; i < MipCount; i++)
		{
			MipBufferData[i - 1] = BufferData;
			MipBufferData[i - 1].TextureIndex = TargetSrvs[i - 1];

			HeapProp = CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_UPLOAD );
			D3D12_RESOURCE_DESC SliceBufferDesc = CD3DX12_RESOURCE_DESC::Buffer( 512 );
			Device->CreateCommittedResource(&HeapProp, D3D12_HEAP_FLAG_NONE, &SliceBufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(MipBuffers[i - 1].GetAddressOf()) );
#if D3D12_Debug_INFO
			MipBuffers[i - 1]->SetName(L"CB_Texture2DToCubeMapSlice");
#endif
			uint32 BufferIndex = AllocateSrv();

			D3D12_CONSTANT_BUFFER_VIEW_DESC BufferDesc = {};
			BufferDesc.BufferLocation = MipBuffers[i - 1]->GetGPUVirtualAddress();
			BufferDesc.SizeInBytes = 512;
			Renderer::Get()->GetD3D12Device()->CreateConstantBufferView(&BufferDesc, SrvCpuHandle(BufferIndex));

			{
				UINT8* ConstantBufferStart;
				CD3DX12_RANGE readRange( 0, 0 );
				MipBuffers[i - 1]->Map(0, &readRange, reinterpret_cast<void**>( &ConstantBufferStart ) );
				memcpy( ConstantBufferStart, &MipBufferData[i-1], sizeof(SliceData));
				MipBuffers[i - 1]->Unmap(0, nullptr);
			}

			CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, BufferIndex, 0);

			for (int32 j = 0; j < 6; j++)
			{
				uint32 SubresourceIndex = D3D12CalcSubresource(i - 1, j, 0, MipCount, 6);
				CD3DX12_RESOURCE_BARRIER Barrier = CD3DX12_RESOURCE_BARRIER::Transition(TextureCubeTarget.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE, SubresourceIndex);
				CommandList->GetD3D12CommandList()->ResourceBarrier(1, &Barrier);
			}

			uint32 MipFaceSize = FaceSize / std::pow(2, i);
			D3D12_VIEWPORT MipViewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(MipFaceSize), static_cast<float>(MipFaceSize));
			CommandList->GetD3D12CommandList()->RSSetViewports(1, &MipViewport);
			CommandList->GetD3D12CommandList()->OMSetRenderTargets(1, &TargetHandles[i], true, NULL);

			CommandList->SetGraphicPipelineState(SlicePso->m_MipPSO);
			CommonResources::Get()->m_UniformCube->BindAndDraw(CommandList);
		}

// -----------------------------------------------------------------------------------------------------------------------------------------

		for (int32 i = 0; i < MipCount-1; i++)
		{
			for (int32 j = 0; j < 6; j++)
			{
				uint32 SubresourceIndex = D3D12CalcSubresource(i, j, 0, MipCount, 6);
				CD3DX12_RESOURCE_BARRIER Barrier = CD3DX12_RESOURCE_BARRIER::Transition(TextureCubeTarget.Get(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_SOURCE, SubresourceIndex);
				CommandList->GetD3D12CommandList()->ResourceBarrier(1, &Barrier);
			}
		}

		for (int32 j = 0; j < 6; j++)
		{
			uint32 SubresourceIndex = D3D12CalcSubresource(MipCount - 1, j, 0, MipCount, 6);
			CD3DX12_RESOURCE_BARRIER Barrier = CD3DX12_RESOURCE_BARRIER::Transition(TextureCubeTarget.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE, SubresourceIndex);
			CommandList->GetD3D12CommandList()->ResourceBarrier(1, &Barrier);
		}

		HeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK);
		Microsoft::WRL::ComPtr<ID3D12Resource> ReadbackBuffers;

		const uint32 SubreourceCount = 6 * MipCount;

		uint64 TextureMemorySize = 0;
		std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> Layouts;
		Layouts.resize(SubreourceCount);

		std::vector<uint32> NumRows;
		NumRows.resize(SubreourceCount);

		std::vector<uint64> RowSizeInBytes;
		RowSizeInBytes.resize(SubreourceCount);

		Device->GetCopyableFootprints(&TargetDesc, 0, SubreourceCount, 0, Layouts.data(), NumRows.data(), RowSizeInBytes.data(), &TextureMemorySize);

		D3D12_RESOURCE_DESC ReadbackDesc = CD3DX12_RESOURCE_DESC::Buffer(TextureMemorySize);
		Device->CreateCommittedResource( &HeapProp, D3D12_HEAP_FLAG_NONE, &ReadbackDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(ReadbackBuffers.GetAddressOf()));
#if D3D12_Debug_INFO
		ReadbackBuffers->SetName(L"CB_Texture2DToTextureCubeReadback");
#endif

		for (int32 i = 0; i < SubreourceCount; i++)
		{
			CD3DX12_TEXTURE_COPY_LOCATION SourceLoc( TextureCubeTarget.Get(), i );
			CD3DX12_TEXTURE_COPY_LOCATION DestLoc( ReadbackBuffers.Get(), Layouts[i] );
			CommandList->GetD3D12CommandList()->CopyTextureRegion( &DestLoc, 0, 0, 0, &SourceLoc, nullptr );
		}

// -----------------------------------------------------------------------------------------------------------------------------------------

		CommandList->GetD3D12CommandList()->Close();

		Renderer::Get()->StartGpuFrameCapture();

		ID3D12CommandList* const Lists[] = { CommandList->GetD3D12CommandList() };
		Renderer::Get()->GetCommandQueue()->ExecuteCommandLists( 1, Lists);
		Renderer::Get()->Flush();

		Renderer::Get()->EndGpuFrameCapture();

// -----------------------------------------------------------------------------------------------------------------------------------------

		UINT8* MemoryStart;

		D3D12_RANGE ReadRange = {};
		ReadRange.Begin = 0;
		ReadRange.End = TextureMemorySize;
		ReadbackBuffers->Map(0, &ReadRange, reinterpret_cast<void**>(&MemoryStart));

		const bool HasCompression = TextureAsset->GetCompression() != ETextureCompression::NoCompression;
		if (HasCompression)
		{
			ScratchImage RawImage;
			RawImage.Initialize2D(MetaData.format, FaceSize, FaceSize, 6, MipCount);

			MetaData.format = TextureHelper::ResolveTextureFormat(MetaData.format, TextureAsset->GetCompression());

			for (int32 i = 0; i < SubreourceCount; i++)
			{
				uint32 MipLevel;
				uint32 ArrayLevel;
				uint32 PlaneLevel;
				D3D12DecomposeSubresource(i, MipCount, 6, MipLevel, ArrayLevel, PlaneLevel);

				const DirectX::Image* SubresourceImage = RawImage.GetImage(MipLevel, ArrayLevel, 0);

				const uint64 RowSize = RowSizeInBytes[i];
				for ( uint64 j = 0; j < NumRows[i]; j++ )
				{
					BYTE* CopyDest = (BYTE*)SubresourceImage->pixels + SubresourceImage->rowPitch * j;
					BYTE* CopySource = MemoryStart + Layouts[i].Offset + Layouts[i].Footprint.RowPitch * j;
					const uint64 CopySize = SubresourceImage->rowPitch;

					memcpy( CopyDest, CopySource, CopySize );
				}
			}

			ScratchImage CompressedImage;
			CoInitializeEx( nullptr, COINITBASE_MULTITHREADED );

			if (MetaData.format != DXGI_FORMAT_BC6H_UF16)
			{
				// TODO: log errors
				HRESULT Res = Compress(RawImage.GetImages(), RawImage.GetImageCount(), RawImage.GetMetadata(),
					MetaData.format, TEX_COMPRESS_DEFAULT, TEX_THRESHOLD_DEFAULT, CompressedImage);
			}
			else
			{
				Microsoft::WRL::ComPtr<ID3D11Device> D11Device;

				Microsoft::WRL::ComPtr<IDXGIFactory2> pFactory;
				HRESULT hr = CreateDXGIFactory1( IID_PPV_ARGS(pFactory.GetAddressOf()));

				IDXGIAdapter* pAdapter = nullptr;
				IDXGIAdapter* TargetAdapter = nullptr;
				UINT adapterIndex = 0;
				uint64 MaxMemory = 0;
				while (pFactory->EnumAdapters(adapterIndex, &pAdapter) != DXGI_ERROR_NOT_FOUND)
				{
					adapterIndex++;
					DXGI_ADAPTER_DESC DDD;
					pAdapter->GetDesc(&DDD);
					if (DDD.DedicatedVideoMemory > MaxMemory)
					{
						MaxMemory = DDD.DedicatedVideoMemory;
						*TargetAdapter = *pAdapter;
					}
				}

				Microsoft::WRL::ComPtr<ID3D11DeviceContext> pContext;
				D3D_FEATURE_LEVEL featureLevel;
				UINT createDeviceFlags = 0;
#ifdef _DEBUG
				createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
				HRESULT WW = D3D11CreateDevice(
					TargetAdapter,
					D3D_DRIVER_TYPE_HARDWARE,
					nullptr,
					createDeviceFlags,
					nullptr,
					0,
					D3D11_SDK_VERSION,
					D11Device.GetAddressOf(),
					&featureLevel,
					pContext.GetAddressOf()
				);

				HRESULT Res = Compress( D11Device.Get(), RawImage.GetImages(), RawImage.GetImageCount(), RawImage.GetMetadata(),
					MetaData.format, TEX_COMPRESS_DEFAULT, TEX_THRESHOLD_DEFAULT, CompressedImage);
			}

			D3D12_RESOURCE_DESC Desc1 = CD3DX12_RESOURCE_DESC::Tex2D(MetaData.format, FaceSize, FaceSize, 6, MipCount);
			Renderer::Get()->GetD3D12Device()->GetCopyableFootprints(&Desc1, 0, SubreourceCount, 0, Layouts.data(), NumRows.data(), RowSizeInBytes.data(), &TextureMemorySize );

			TextureAsset->ReleaseImageBlobs();
			D3DCreateBlob(TextureMemorySize, &TextureAsset->m_ImageBlob);

			for (int32 i = 0; i < SubreourceCount; i++)
			{
				uint32 MipLevel;
				uint32 ArrayLevel;
				uint32 PlaneLevel;
				D3D12DecomposeSubresource(i, MipCount, 6, MipLevel, ArrayLevel, PlaneLevel);
				const DirectX::Image* MipSource = CompressedImage.GetImage( MipLevel, ArrayLevel, 0 );

				for ( uint64 j = 0; j < NumRows[i]; j++ )
				{
					BYTE* CopyDest = (BYTE*)TextureAsset->m_ImageBlob->GetBufferPointer() + Layouts[i].Offset + Layouts[i].Footprint.RowPitch * j;
					BYTE* CopySource = (BYTE*)MipSource->pixels + MipSource->rowPitch * j;
					const uint64 CopySize   = MipSource->rowPitch;

					memcpy( CopyDest, CopySource, CopySize );
				}
			}
		}

		else
		{
			TextureAsset->ReleaseImageBlobs();
			D3DCreateBlob(TextureMemorySize, &TextureAsset->m_ImageBlob);
			memcpy(TextureAsset->m_ImageBlob->GetBufferPointer(), MemoryStart, TextureMemorySize);
		}
		ReadbackBuffers->Unmap(0, nullptr);

// -----------------------------------------------------------------------------------------------------------------------------------------

		TextureAsset->m_SizeX = FaceSize;
		TextureAsset->m_SizeY = FaceSize;
		TextureAsset->m_Format = TextureAsset->IsSRGB() ? MakeSRGB(MetaData.format) : MetaData.format;
		TextureAsset->m_MipLevels = MipCount;
		TextureAsset->MarkRenderStateDirty();
	}

}

#endif