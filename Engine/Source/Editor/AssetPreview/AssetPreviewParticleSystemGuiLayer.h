#pragma once

#if WITH_EDITOR

#include "ForwardTypes.h"
#include "Runtime/Renderer/ImGui/ImGuiLayer.h"
#include "Runtime/Particle/ParticleSystem.h"

namespace Drn
{
	class ViewportPanel;
	class ParticleSystem;

	class AssetPreviewParticleSystemGuiLayer : public ImGuiLayer
	{
	public:
		AssetPreviewParticleSystemGuiLayer(ParticleSystem* InOwningAsset);
		~AssetPreviewParticleSystemGuiLayer();

		virtual void Draw( float DeltaTime ) override;

		void DrawViewport(float DeltaTime);
		void DrawParticleParams();
		void DrawEmitterParams();

	private:

		TRefCountPtr<class PreviewWorld> m_World;
		std::unique_ptr<ViewportPanel> m_ViewportPanel;

		class Particle* ParticlePreview;

		AssetHandle<ParticleSystem> m_OwningAsset;
	};
}

#endif