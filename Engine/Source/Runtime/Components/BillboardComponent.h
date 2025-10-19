#pragma once

#include "ForwardTypes.h"
#include "Runtime/Engine/PrimitiveComponent.h"
#include "Runtime/Engine/PrimitiveSceneProxy.h"

namespace Drn
{
	struct BillboardData
	{
	public:
		BillboardData() = default;

		Matrix m_LocalToProjetcion;
		Guid m_Guid;
		uint32 m_TextureIndex;
	};

	class BillboardSceneProxy : public PrimitiveSceneProxy
	{
	public:
		BillboardSceneProxy( class BillboardComponent* InBillboardComponent );
		virtual ~BillboardSceneProxy();

		inline void SetSprite( AssetHandle<Texture2D> InSprite ) { m_Sprite = InSprite; }

		virtual void RenderMainPass( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer ) override;
		virtual void RenderPrePass( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer ) override;
		virtual void RenderShadowPass(ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer, LightSceneProxy* LightProxy) override;
		virtual void RenderDecalPass(ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer) override;


#if WITH_EDITOR
		virtual void RenderHitProxyPass( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer ) override;
		virtual void RenderSelectionPass( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer ) override;
		virtual void RenderEditorPrimitivePass( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer ) override;
#endif

		virtual void InitResources(ID3D12GraphicsCommandList2* CommandList) override;
		virtual void UpdateResources(ID3D12GraphicsCommandList2* CommandList) override;

		virtual void SetConstantAndSrv(ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer);

		virtual PrimitiveComponent* GetPrimitive() override;

	protected:
		AssetHandle<Texture2D> m_Sprite;
		Guid m_Guid;

		BillboardData m_BillboardData;
		Resource* m_BillboardBuffer;

		class BillboardComponent* m_BillboardComponent;
	};

	class BillboardComponent : public PrimitiveComponent
	{
	public:
		BillboardComponent();
		virtual ~BillboardComponent();

		virtual void Serialize( Archive& Ar ) override;

		void SetSprite(AssetHandle<Texture2D> NewSprite);
		inline AssetHandle<Texture2D> GetSprite() const { return m_Sprite; }

		virtual void RegisterComponent(World* InOwningWorld) override;
		virtual void UnRegisterComponent() override;

#if WITH_EDITOR
		inline virtual bool ShouldDrawInComponentHeirarchy() const override { return false; }
#endif

	protected:
		AssetHandle<Texture2D> m_Sprite;

		BillboardSceneProxy* m_BillboardSceneProxy;
	};
}