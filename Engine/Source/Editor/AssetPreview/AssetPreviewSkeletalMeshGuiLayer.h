#pragma once

#include "ForwardTypes.h"

#if WITH_EDITOR

#include "Runtime/Renderer/ImGui/ImGuiLayer.h"
#include "Editor/Misc/ViewportCameraInputHandler.h"

namespace Drn
{
	class AssetPreviewStaticMesh;
	class ViewportPanel;

	class AssetPreviewSkeletalMeshGuiLayer : public ImGuiLayer
	{
	public:
		AssetPreviewSkeletalMeshGuiLayer(SkeletalMesh* InOwningAsset);
		~AssetPreviewSkeletalMeshGuiLayer();

		virtual void Draw( float DeltaTime ) override;

		void SetCurrentFocus();

	protected:

		void DrawMenu();
		void DrawDetailPanel();
		void ShowSourceFileSelection();

		void OnSelectedSourceFile( std::string FilePath );

		void DrawSkeletonTree();
		void DrawSkeletonTreeNode(int32 NodeIndex);

		void DrawDebugs();
		float m_DebugLinesSize;

		bool m_DrawNormals;
		bool m_DrawTangents;
		bool m_DrawBitTangents;

		bool m_DrawBounds;
		bool m_PreviewWeights;

		int32 SelectedBoneIndex;

		World* PreviewWorld;
		SkeletalMeshActor* PreviewMesh;
		class SkyLightActor* m_SkyLight;
		class DirectionalLightActor* m_DirectionalLight;

		AssetHandle<SkeletalMesh> m_OwningAsset;
		std::unique_ptr<ViewportPanel> m_ViewportPanel;

		TRefCountPtr<MaterialInstanceDynamic> BoneWeightMaterial;

		bool m_ShowSceneSetting;
		bool m_ShowDetail;

	private:
		
	};
}
#endif