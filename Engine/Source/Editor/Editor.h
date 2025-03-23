#pragma once

#if WITH_EDITOR

#include "ForwardTypes.h"

LOG_DECLARE_CATEGORY(LogEditor);

namespace Drn
{

	class Editor
	{
	public:
		Editor();
		~Editor();

		void Init();
		void Tick(float DeltaTime);
		void Shutdown();

		static Editor* Get();


	protected:

		
		// @TODO: link list
		std::vector<std::unique_ptr<AssetPreview>> AssetPreviews;

	private:

		void OnSelectedFile(const std::string Path);
		void OpenAssetView(const std::string Path);
		
		
		static std::unique_ptr<Editor> SingletonInstance;

		friend class ContentBrowserGuiLayer;
	};

}

#endif