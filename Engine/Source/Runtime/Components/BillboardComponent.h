#pragma once

#include "ForwardTypes.h"
#include "Runtime/Engine/PrimitiveComponent.h"
#include "Runtime/Engine/PrimitiveSceneProxy.h"

namespace Drn
{
	class BillboardSceneProxy : public PrimitiveSceneProxy
	{
	public:
		BillboardSceneProxy( class BillboardComponent* InBillboardComponent );
		virtual ~BillboardSceneProxy();

		inline void SetSprite( AssetHandle<Texture2D> InSprite ) { m_Sprite = InSprite; }

		virtual void RenderMainPass( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer ) override;
#if WITH_EDITOR
		virtual void RenderHitProxyPass( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer ) override;
		virtual void RenderSelectionPass( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer ) override;
		virtual void RenderEditorPrimitivePass( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer ) override;
#endif

		virtual void InitResources(ID3D12GraphicsCommandList2* CommandList) override;
		virtual void UpdateResources(ID3D12GraphicsCommandList2* CommandList) override;

		virtual PrimitiveComponent* GetPrimitive() override;

	protected:
		AssetHandle<Texture2D> m_Sprite;

		class BillboardComponent* m_BillboardComponent;
	};

	class BillboardComponent : public PrimitiveComponent
	{
	public:
		BillboardComponent();
		virtual ~BillboardComponent();

		void SetSprite(AssetHandle<Texture2D> NewSprite);
		inline AssetHandle<Texture2D> GetSprite() const { return m_Sprite; }

		virtual void RegisterComponent(World* InOwningWorld) override;
		virtual void UnRegisterComponent() override;

	protected:
		AssetHandle<Texture2D> m_Sprite;

		BillboardSceneProxy* m_BillboardSceneProxy;
	};
}