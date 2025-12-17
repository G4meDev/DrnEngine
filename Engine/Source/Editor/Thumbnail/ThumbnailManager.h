#pragma once

#if WITH_EDITOR

#include "ForwardTypes.h"

#include "Runtime/Engine/PreviewWorld.h"

namespace Drn
{
	struct ThumbnailCaptureEvent : RefCountedObject
	{
		ThumbnailCaptureEvent(class SceneRenderer* InTargetScene, const std::string& InPath)
			: TargetScene(InTargetScene)
			, Path(InPath)
			, FenceValue(0)
			, bInitalized(false)
		{}

		TRefCountPtr<class RenderTexture2D> ReadbackBuffer;
		class SceneRenderer* TargetScene;
		std::string Path;
		uint64 FenceValue;
		bool bInitalized;

		TRefCountPtr<class PreviewWorld> m_PreviewWorld;
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

		ThumbnailCaptureEvent* CaptureSceneThumbnail(class SceneRenderer* TargetScene, const std::string& Path);
		inline bool IsCaptureThumbnailAllowed()
		{
			if (RemainingAllowedCaptureForFrame > 0)
			{
				RemainingAllowedCaptureForFrame.fetch_add(-1);
				return true;
			}

			return false;
		};

		void ProccessRequestedThumbnails(class D3D12CommandList* CmdList);

		class RenderTexture2D* GetThumbnailWithPath(const std::string& Path);

		void Flush();

	private:
		static TRefCountPtr<ThumbnailManager> SingletonInstance;

		std::string AssetPathToThumbnailPath(const std::string& AssetPath);

		void ProccessCaptureQueue();
		void WriteCaptureToDisk(const ThumbnailCaptureEvent& Event);

		void GenerateThumbnailForAssetPath(const std::string& AssetPath);


		std::vector<TRefCountPtr<ThumbnailCaptureEvent>> CaptureQueue;

		std::unordered_map<std::string, TRefCountPtr<RenderTexture2D>> ThumbnailPool;
		std::vector<std::string> RequestedThumbnails;

		std::atomic<uint32> RemainingAllowedCaptureForFrame;
	};

}


#endif