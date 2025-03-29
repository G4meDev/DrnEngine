#pragma once

#if WITH_EDITOR

#include "Runtime/Renderer/ImGui/ImGuiLayer.h"

LOG_DECLARE_CATEGORY(LogStaticMeshPreview);

namespace Drn
{
	class AssetPreviewStaticMesh;

	class AssetPreviewStaticMeshGuiLayer : public ImGuiLayer
	{
	public:
		AssetPreviewStaticMeshGuiLayer(StaticMesh* InOwningAsset);
		~AssetPreviewStaticMeshGuiLayer();

		virtual void Draw() override;

		void SetCurrentFocus();

		void DrawSidePanel();

		void DrawViewport();

		void OnViewportSizeChanged( const IntPoint& NewSize );

	protected:

		D3D12_CPU_DESCRIPTOR_HANDLE ViewCpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE ViewGpuHandle;

		IntPoint CachedViewportSize = IntPoint( 1920, 1080 );

		World* PreviewWorld;
		Scene* PreviewScene;
		SceneRenderer* MainView;

		StaticMeshComponent* PreviewMesh;

		AssetHandle<StaticMesh> m_OwningAsset;
	private:
	};
}
#endif