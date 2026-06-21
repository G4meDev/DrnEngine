#pragma once

#if WITH_EDITOR

#include "ForwardTypes.h"
#include "Runtime/Renderer/ImGui/ImGuiLayer.h"
#include "Runtime/Physic/PhysicalMaterial.h"

namespace Drn
{
	class ViewportPanel;
	class PhysicalMaterial;

	class AssetPreviewPhysicalMaterialGuiLayer : public ImGuiLayer
	{
	public:
		AssetPreviewPhysicalMaterialGuiLayer(PhysicalMaterial* InOwningAsset);
		~AssetPreviewPhysicalMaterialGuiLayer();

		virtual void Draw( float DeltaTime ) override;

	private:

		AssetHandle<PhysicalMaterial> m_OwningAsset;
	};
}

#endif