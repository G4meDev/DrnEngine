#pragma once

#if WITH_EDITOR

#include "ForwardTypes.h"

LOG_DECLARE_CATEGORY(LogEditor);

namespace Drn
{
	class FileImportMenu;

	class Editor
	{
	public:
		Editor();
		~Editor();

		void Init();
		void Tick(float DeltaTime);
		void Shutdown();

		static Editor* Get();

		std::shared_ptr<FileImportMenu> OpenImportMenu();
		void CloseImportMenu();

	protected:

		std::vector<std::unique_ptr<AssetPreview>> m_AssetPreviews;
		std::shared_ptr<FileImportMenu> m_FileImportMenu;

	private:

		void OnSelectedFile(const std::string Path);
		void OpenAssetView(const std::string Path);

		void CloseAssetPreviews();

		static std::unique_ptr<Editor> SingletonInstance;

		friend class ContentBrowserGuiLayer;
	};

}

#endif