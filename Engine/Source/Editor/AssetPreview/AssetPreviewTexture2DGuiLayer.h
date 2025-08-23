#pragma once

#if WITH_EDITOR

#include "ForwardTypes.h"
#include "Runtime/Renderer/ImGui/ImGuiLayer.h"

namespace Drn
{
	class ViewportPanel;

	class AssetPreviewTexture2DGuiLayer : public ImGuiLayer
	{
	public:
		AssetPreviewTexture2DGuiLayer(Texture2D* InOwningAsset);
		~AssetPreviewTexture2DGuiLayer();

		virtual void Draw( float DeltaTime ) override;

	protected:

		void DrawMenu();
		void DrawDetailsPanel();

		void UpdateMipLevel();
		void UpdateShowColor();

	private:

		World* m_PreviewWorld;
		StaticMeshActor* m_PreviewMeshPlane;

		AssetHandle<Texture2D> m_OwningAsset;
		std::unique_ptr<ViewportPanel> m_ViewportPanel;

		AssetHandle<Material> m_PreviewMaterial;
		float m_MipLevel;

		bool m_ShowR = true;
		bool m_ShowG = true;
		bool m_ShowB = true;
		bool m_ShowA = false;
	};
}

#endif