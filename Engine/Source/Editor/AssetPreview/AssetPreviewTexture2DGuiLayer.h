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
		void DrawViewportPanel();
		void DrawDetailsPanel();

		void AllocateHandles();
		void ReleaseHandles();

	private:

		AssetHandle<Texture2D> m_OwningAsset;

		D3D12_CPU_DESCRIPTOR_HANDLE ViewCpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE ViewGpuHandle;
	};
}

#endif