#pragma once

#if WITH_EDITOR

#include "ForwardTypes.h"
#include "Runtime/Renderer/ImGui/ImGuiLayer.h"
#include "Runtime/Particle/ParticleSystem.h"

namespace Drn
{
	class ViewportPanel;
	class ParticleSystem;
	class ParticleEmitter;

	class AssetPreviewParticleSystemGuiLayer : public ImGuiLayer
	{
	public:
		AssetPreviewParticleSystemGuiLayer(ParticleSystem* InOwningAsset);
		~AssetPreviewParticleSystemGuiLayer();

		virtual void Draw( float DeltaTime ) override;

		void DrawViewport(float DeltaTime);
		void DrawParticleParams();
		void DrawEmitterParams();
		void DrawModuleParams();

		void DrawEmitters();
		void DrawEmitterHeader(int32 Index);
		void DrawEmitterModules(int32 Index, bool EmitterSelected);

		int32 GetModuleStackIndex(ParticleEmitter* Emitter, EParticleModuleStage Stage, int32 InternalIndex);
		void GetModuleInternalIndex(ParticleEmitter* Emitter, int32 StackIndex, EParticleModuleStage& Stage, int32& InternalIndex);

	private:

		int32 SelectedEmitterIndex;
		int32 DeferrEmitterDeleteIndex;

		int32 SelectedModuleIndex;
		int32 DeferrModuleDeleteIndex;
		int32 DeferrModuleEmitterDeleteIndex;

		TRefCountPtr<class PreviewWorld> m_World;
		std::unique_ptr<ViewportPanel> m_ViewportPanel;

		class Particle* ParticlePreview;

		AssetHandle<ParticleSystem> m_OwningAsset;
	};


}

#endif