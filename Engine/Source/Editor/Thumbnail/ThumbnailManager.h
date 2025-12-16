#pragma once

#if WITH_EDITOR

#include "ForwardTypes.h"

namespace Drn
{
	struct ThumbnailCaptureEvent : RefCountedObject
	{
		ThumbnailCaptureEvent(class SceneRenderer* InTargetScene, const std::string& InPath)
			: TargetScene(InTargetScene)
			, Path(InPath)
			, FenceValue(0)
		{}

		TRefCountPtr<class RenderTexture2D> ReadbackBuffer;
		class SceneRenderer* TargetScene;
		std::string Path;
		uint64 FenceValue;
	};

	class ThumbnailManager : public RefCountedObject
	{
	public:
		ThumbnailManager();
		~ThumbnailManager();

		void Init();
		void Tick( float DeltaTime );
		void Shutdown();

		static ThumbnailManager* Get();

		void CaptureSceneThumbnail(class SceneRenderer* TargetScene, const std::string& Path);
		inline bool IsCaptureThumbnailAllowed()
		{
			if (RemainingAllowedCaptureForFrame > 0)
			{
				RemainingAllowedCaptureForFrame.fetch_add(-1);
				return true;
			}

			return false;
		};

		std::vector<TRefCountPtr<ThumbnailCaptureEvent>> CaptureQueue;

	private:
		static TRefCountPtr<ThumbnailManager> SingletonInstance;

		void ProccessCaptureQueue();
		void WriteCaptureToDisk(const ThumbnailCaptureEvent& Event);

		std::atomic<uint32> RemainingAllowedCaptureForFrame;
	};

}


#endif