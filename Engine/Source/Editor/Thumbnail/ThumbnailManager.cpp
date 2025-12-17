#include "DrnPCH.h"
#include "ThumbnailManager.h"

#if WITH_EDITOR

#include <DirectXTex.h>

#define MAX_CAPTURE_PER_FRAME 2
#define THUMBNAIL_TEXTURE_SIZE 256

namespace Drn
{
	TRefCountPtr<ThumbnailManager> ThumbnailManager::SingletonInstance = nullptr;
	ThumbnailManager::ThumbnailManager() {}
	ThumbnailManager::~ThumbnailManager() {}

	void ThumbnailManager::Init()
	{
		
	}

	void ThumbnailManager::Tick( float DeltaTime )
	{
		RemainingAllowedCaptureForFrame.store(MAX_CAPTURE_PER_FRAME);

		ProccessCaptureQueue();
	}

	void ThumbnailManager::Shutdown()
	{
		
	}

	ThumbnailManager* ThumbnailManager::Get()
	{
		if (!SingletonInstance)
		{
			SingletonInstance = new ThumbnailManager;
		}

		return SingletonInstance;
	}

	void ThumbnailManager::CaptureSceneThumbnail( class SceneRenderer* TargetScene, const std::string& Path )
	{
		const std::string ThumbnailPath = Path::GetThumbnailPath() + Path::RemoveFileExtension(Path) + ".tga";
		drn_check( TargetScene && ThumbnailPath != NAME_NULL );

		CaptureQueue.emplace_back(new ThumbnailCaptureEvent(TargetScene, ThumbnailPath));
		TargetScene->ThumbnailCaptureEvents.push_back(CaptureQueue.back());
	}

	void ThumbnailManager::ProccessCaptureQueue()
	{
		CaptureQueue.erase(std::remove_if(CaptureQueue.begin(), CaptureQueue.end(), [this](TRefCountPtr<ThumbnailCaptureEvent>& Event)
		{
			bool bCompleted = Event->bInitalized && Renderer::Get()->GetFence()->IsFenceComplete(Event->FenceValue);

			if (bCompleted)
			{
				WriteCaptureToDisk(*Event);
			}

			return bCompleted;
		})
		, CaptureQueue.end());
	}

	void ThumbnailManager::WriteCaptureToDisk(const ThumbnailCaptureEvent& Event)
	{
		const std::string ThumbnailDirectory = Path::GetDirectory(Event.Path);
		drn_check(ThumbnailDirectory != NAME_NULL);

		FileSystem::CreateDirectoryIfDoesntExist(ThumbnailDirectory);

		RenderTexture2D* CapturedTexture = Event.ReadbackBuffer;

		uint32 TextureWidth = CapturedTexture->GetSizeX();
		uint32 TextureHeight = CapturedTexture->GetSizeY();

		uint8* Bytes = new uint8[TextureWidth * TextureHeight * 4];
		DirectX::Image Image;
		Image.pixels = Bytes;
		Image.width = TextureWidth;
		Image.height = TextureHeight;
		Image.format = DISPLAY_OUTPUT_FORMAT;

		ComputePitch(Image.format, Image.width, Image.height, Image.rowPitch, Image.slicePitch);

		D3D12_PLACED_SUBRESOURCE_FOOTPRINT Footprint;
		uint32 Rows;
		uint64 RowSizeInBytes;
		uint64 Size = 0;

		const D3D12_RESOURCE_DESC& ResourceDesc = CapturedTexture->GetResource()->GetDesc();
		D3D12_RESOURCE_DESC Desc = CD3DX12_RESOURCE_DESC::Tex2D(CapturedTexture->GetFormat(), CapturedTexture->GetSizeX(), CapturedTexture->GetSizeY());

		CapturedTexture->GetParentDevice()->GetD3D12Device()->GetCopyableFootprints(&Desc, 0, 1, 0, &Footprint, &Rows, &RowSizeInBytes, &Size);

		uint8* MappedAddress = (uint8*)CapturedTexture->m_ResourceLocation.GetMappedBaseAddress();

		for ( uint64 Row = 0; Row < Rows; Row++ )
		{
			BYTE* CopyDest = (BYTE*)Bytes + Image.rowPitch * Row;
			BYTE* CopySource = MappedAddress + Footprint.Offset + Footprint.Footprint.RowPitch * Row;
			const uint64 CopySize = Image.rowPitch;

			memcpy( CopyDest, CopySource, CopySize );
		}

		CoInitializeEx(nullptr, COINIT_MULTITHREADED);

		DirectX::ScratchImage Result;
		Resize(Image, THUMBNAIL_TEXTURE_SIZE, THUMBNAIL_TEXTURE_SIZE, DirectX::TEX_FILTER_DEFAULT, Result);

		SaveToTGAFile( *Result.GetImages(), DirectX::TGA_FLAGS_NONE, StringHelper::s2ws( Event.Path ).c_str() );
		delete[] Bytes;

		CoUninitialize();
	}

	void ThumbnailManager::ProccessRequestedThumbnails( D3D12CommandList* CmdList )
	{
		CoInitializeEx(nullptr, COINIT_MULTITHREADED);

		for (const std::string& Path : RequestedThumbnails)
		{
			if (FileSystem::FileExists(Path))
			{
				DirectX::ScratchImage Image;
				DirectX::TexMetadata MetaData;
			
				LoadFromTGAFile(StringHelper::s2ws(Path).c_str(), &MetaData, Image);
			
				RenderResourceCreateInfo CreateInfo( Image.GetImages()->pixels, nullptr, ClearValueBinding::Black, "Tex_Thumbnail" );
				RenderTexture2D* Result = RenderTexture2D::Create(CmdList, MetaData.width, MetaData.height, MetaData.format, 1, 1, false,
					(ETextureCreateFlags)(ETextureCreateFlags::ShaderResource), CreateInfo);

				ThumbnailPool[Path] = Result;

				// TODO: improve / remove
				CmdList->AddTransitionBarrier(Result->GetResource(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
				CmdList->FlushBarriers();
			}
		}

		RequestedThumbnails.clear();

		CoUninitialize();
	}

	RenderTexture2D* ThumbnailManager::GetThumbnailWithPath( const std::string& Path )
	{
		const std::string ThumbnailPath = Path::GetThumbnailPath() + Path::RemoveFileExtension(Path) + ".tga";

		if (!FileSystem::FileExists(ThumbnailPath))
		{
			return nullptr;
		}

		auto it = ThumbnailPool.find(ThumbnailPath);
		if (it != ThumbnailPool.end())
		{
			return it->second;
		}

		RequestedThumbnails.push_back(ThumbnailPath);
		return nullptr;
	}

	void ThumbnailManager::Flush()
	{
		ThumbnailPool.clear();
		RequestedThumbnails.clear();
	}

        }

#endif