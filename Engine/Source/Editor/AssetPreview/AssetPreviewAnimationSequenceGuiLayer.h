#pragma once

#include "ForwardTypes.h"

#if WITH_EDITOR

#include "Runtime/Renderer/ImGui/ImGuiLayer.h"
#include "Editor/Misc/ViewportCameraInputHandler.h"

namespace Drn
{
	class AssetPreviewStaticMesh;
	class ViewportPanel;

	class AssetPreviewAnimationSequenceGuiLayer : public ImGuiLayer
	{
	public:
		AssetPreviewAnimationSequenceGuiLayer(AnimationSequence* InOwningAsset);
		~AssetPreviewAnimationSequenceGuiLayer();

		virtual void Draw( float DeltaTime ) override;

		void OnReimport();

	protected:

		void DrawMenu();
		void DrawDetailPanel();
		void ShowSourceFileSelection();

		void OnSelectedSourceFile( std::string FilePath );

		void DrawSkeletonTree();
		void DrawSkeletonTreeNode(int32 NodeIndex);

		void OnSelectedNewComponent( const HitProxyData& Data );

		void GetGizmoTransform( bool& bDrawGizmo, Transform& GizmoTransform );
		void OnGizmoTransformChanged( const Transform& GizmoTransform, EGizmoSpace GizmoSpace );

		int32 SelectedBoneIndex;

		World* PreviewWorld;
		SkeletalMeshActor* PreviewMesh;
		class SkyLightActor* m_SkyLight;
		class DirectionalLightActor* m_DirectionalLight;

		AssetHandle<AnimationSequence> m_OwningAsset;
		std::unique_ptr<ViewportPanel> m_ViewportPanel;

		bool m_ShowSceneSetting;
		bool m_ShowDetail;

		float PreviewSpeed = 0.1f;
		bool StepAnimation = true;
		int32 DisplayFrameNumber;

		friend class AnimatorSkeletalMeshPreview;
		friend class AnimatorAnimationSequencePreview;

	private:
		
	};
}
#endif