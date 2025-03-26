#pragma once

#if WITH_EDITOR

#include "Runtime/Renderer/ImGui/ImGuiLayer.h"

namespace Drn
{
	class AssetPreviewStaticMesh;

	class AssetPreviewStaticMeshGuiLayer : public ImGuiLayer
	{
	public:
		AssetPreviewStaticMeshGuiLayer(AssetPreviewStaticMesh* InOwningAsset);

		virtual void Draw() override;

		void SetCurrentFocus();

		void DrawSidePanel();

	protected:

		World* PreviewWorld;
		Scene* PreviewScene;
		SceneRenderer* MainView;

		AssetPreviewStaticMesh* m_OwningAsset;

	private:
	};
}
#endif