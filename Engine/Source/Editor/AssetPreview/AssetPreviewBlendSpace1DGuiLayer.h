#pragma once

#include "ForwardTypes.h"

#if WITH_EDITOR

#include "Runtime/Renderer/ImGui/ImGuiLayer.h"
#include "Editor/Misc/ViewportCameraInputHandler.h"

namespace Drn
{
	class AssetPreviewStaticMesh;
	class ViewportPanel;

	class AssetPreviewBlendSpace1DGuiLayer : public ImGuiLayer
	{
	public:
		AssetPreviewBlendSpace1DGuiLayer(BlendSpace1D* InOwningAsset);
		~AssetPreviewBlendSpace1DGuiLayer();

		virtual void Draw( float DeltaTime ) override;

	protected:

		void DrawMenu();
		void DrawDetailPanel();
		void DrawSkeleton();
		void DrawSamples();

		World* PreviewWorld;
		SkeletalMeshActor* PreviewMesh;
		class SkyLightActor* m_SkyLight;
		class DirectionalLightActor* m_DirectionalLight;

		AssetHandle<BlendSpace1D> m_OwningAsset;
		std::unique_ptr<ViewportPanel> m_ViewportPanel;

		bool m_ShowSceneSetting;
		bool m_ShowDetail;

		float PreviewSampleTime = 0.0f;

		friend class AnimatorBlendSpace1DPreview;

	private:
		
	};
}
#endif