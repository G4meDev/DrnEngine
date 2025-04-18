#pragma once

#if WITH_EDITOR

#include "Runtime/Renderer/ImGui/ImGuiLayer.h"
#include "Editor/Misc/ViewportCameraInputHandler.h"

LOG_DECLARE_CATEGORY(LogStaticMeshPreview);

namespace Drn
{
	class AssetPreviewStaticMesh;
	class ViewportPanel;

	class AssetPreviewStaticMeshGuiLayer : public ImGuiLayer
	{
	public:
		AssetPreviewStaticMeshGuiLayer(StaticMesh* InOwningAsset);
		~AssetPreviewStaticMeshGuiLayer();

		virtual void Draw( float DeltaTime ) override;

		void SetCurrentFocus();


	protected:

		void DrawMenu();
		void DrawDetailPanel();
		void ShowSourceFileSelection();
		
		void OnSelectedSourceFile( std::string FilePath );

		World* PreviewWorld;
		StaticMeshActor* PreviewMesh;

		AssetHandle<StaticMesh> m_OwningAsset;
		std::unique_ptr<ViewportPanel> m_ViewportPanel;

		bool m_ShowSceneSetting;
		bool m_ShowDetail;

	private:
	};
}
#endif