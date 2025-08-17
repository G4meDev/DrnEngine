#pragma once

#if WITH_EDITOR

#include "ForwardTypes.h"
#include "Runtime/Renderer/ImGui/ImGuiLayer.h"

namespace Drn
{
	class ViewportPanel;

	class AssetPreviewTextureCubeGuiLayer : public ImGuiLayer
	{
	public:
		AssetPreviewTextureCubeGuiLayer(TextureCube* InOwningAsset);
		~AssetPreviewTextureCubeGuiLayer();

		virtual void Draw( float DeltaTime ) override;

	protected:

		void DrawMenu();
		void DrawDetailsPanel();

	private:

		World* m_PreviewWorld;
		StaticMeshActor* m_PreviewMeshPlane;

		AssetHandle<TextureCube> m_OwningAsset;
		std::unique_ptr<ViewportPanel> m_ViewportPanel;
	};
}

#endif