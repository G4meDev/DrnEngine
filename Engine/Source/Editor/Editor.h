#pragma once

#if WITH_EDITOR

#include "ForwardTypes.h"

LOG_DECLARE_CATEGORY(LogEditor);

namespace Drn
{
	class FileImportMenu;
	class TaskGraphVisualizer;

	class Editor
	{
	public:
		Editor();
		~Editor();

		void Init();
		void Tick(float DeltaTime);
		void Shutdown();

		static Editor* Get();

		std::shared_ptr<FileImportMenu> OpenImportMenu(const std::string& InDisplayText, const char* InFilters, std::function<void(std::string)> InOnSelectedFile);
		void CloseImportMenu();

		float SidePanelSize = 300.0f;

		void NotifyMaterialReimported(const AssetHandle<Material>& Mat);

		void OnOpenLevel( World* OpenedWorld );
		void OnCloseLevel( World* ClosedWorld );

		void OpenTaskGraphVisualizer();

		void ReimportMaterials();

	protected:

		std::shared_ptr<FileImportMenu> m_FileImportMenu;

		TaskGraphVisualizer* m_TaskGraphVisualizer;

	private:

		void OnSelectedFile(const std::string Path);

		static std::unique_ptr<Editor> SingletonInstance;

		friend class ContentBrowserGuiLayer;
	};

}

#endif