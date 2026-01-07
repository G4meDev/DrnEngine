#pragma once

#if WITH_EDITOR

#include "ForwardTypes.h"
#include "Runtime/Renderer/ImGui/ImGuiLayer.h"

LOG_DECLARE_CATEGORY(LogAssetPreviewMaterial);

namespace Drn
{
	class ViewportPanel;
	class MaterialInstance;

	class AssetPreviewMaterialInstanceGuiLayer : public ImGuiLayer
	{
	public:
		AssetPreviewMaterialInstanceGuiLayer(MaterialInstance* InOwningAsset);
		~AssetPreviewMaterialInstanceGuiLayer();

		virtual void Draw( float DeltaTime ) override;

	protected:

		void DrawMenu();
		void DrawDetailsPanel();

		void DrawParameters();

	private:

		World* PreviewWorld;
		StaticMeshActor* PreviewMesh;

		class SkyLightActor* m_SkyLight;
		class DirectionalLightActor* m_DirectionalLight;

		AssetHandle<MaterialInstance> m_OwningAsset;
		std::unique_ptr<ViewportPanel> m_ViewportPanel;

		bool m_ShowDetailsPanel;
		bool m_ShowSceneSettingsPanel;
	};
}

#endif