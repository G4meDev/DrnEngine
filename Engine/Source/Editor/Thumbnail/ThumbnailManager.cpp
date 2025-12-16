#include "DrnPCH.h"
#include "ThumbnailManager.h"

#if WITH_EDITOR

#define MAX_CAPTURE_PER_FRAME 2

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
		const std::string ThumbnailPath = Path::GetThumbnailPath() + Path::RemoveFileExtension(Path) + ".png";
		drn_check( TargetScene && ThumbnailPath != NAME_NULL );

		CaptureQueue.emplace_back(new ThumbnailCaptureEvent(TargetScene, ThumbnailPath));

	}

	void ThumbnailManager::ProccessCaptureQueue()
	{
		CaptureQueue.erase(std::remove_if(CaptureQueue.begin(), CaptureQueue.end(), [this](TRefCountPtr<ThumbnailCaptureEvent>& Event)
		{
			bool bCompleted = Renderer::Get()->GetFence()->IsFenceComplete(Event->FenceValue);

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
		std::cout << "yoho!!!\n";
	}

}

#endif