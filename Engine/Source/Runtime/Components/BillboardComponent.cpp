#include "DrnPCH.h"
#include "BillboardComponent.h"
#include "Runtime/Renderer/CommonResources.h"

namespace Drn
{
	BillboardComponent::BillboardComponent()
		: PrimitiveComponent()
		, m_BillboardSceneProxy(nullptr)
	{
		SetEditorPrimitive(true);

	}

	BillboardComponent::~BillboardComponent()
	{
		
	}

	void BillboardComponent::Serialize( Archive& Ar )
	{
		// editor only component. no need to serialize
	}

	void BillboardComponent::SetSprite( AssetHandle<Texture2D> NewSprite )
	{
		m_Sprite = NewSprite;

		if (m_BillboardSceneProxy)
		{
			m_BillboardSceneProxy->SetSprite(m_Sprite);
		}
	}

	void BillboardComponent::RegisterComponent( World* InOwningWorld )
	{
		SceneComponent::RegisterComponent(InOwningWorld);

		m_BillboardSceneProxy = new BillboardSceneProxy(this);
		InOwningWorld->GetScene()->RegisterPrimitiveProxy(m_BillboardSceneProxy);
		m_SceneProxy = m_BillboardSceneProxy;
	}

	void BillboardComponent::UnRegisterComponent()
	{
		if (GetWorld()->GetScene())
		{
			GetWorld()->GetScene()->UnRegisterPrimitiveProxy(m_BillboardSceneProxy);
		}

		m_BillboardSceneProxy = nullptr;
		m_SceneProxy = nullptr;

		SceneComponent::UnRegisterComponent();
	}

// -------------------------------------------------------------------------------------------------------

	BillboardSceneProxy::BillboardSceneProxy( class BillboardComponent* InBillboardComponent )
		: PrimitiveSceneProxy( InBillboardComponent )
		, m_Sprite( InBillboardComponent->GetSprite() )
		, m_Guid( InBillboardComponent->GetParent() ? InBillboardComponent->GetParent()->GetGuid() : InBillboardComponent->GetGuid() )
	{}

	BillboardSceneProxy::~BillboardSceneProxy()
	{}

	void BillboardSceneProxy::RenderMainPass( D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		
	}

	void BillboardSceneProxy::RenderPrePass( D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		
	}

	void BillboardSceneProxy::RenderShadowPass( D3D12CommandList* CommandList, SceneRenderer* Renderer, LightSceneProxy* LightProxy )
	{
		
	}

	void BillboardSceneProxy::RenderDecalPass( D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		
	}

#if WITH_EDITOR
	void BillboardSceneProxy::RenderHitProxyPass( D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		CommandList->SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		CommandList->GetD3D12CommandList()->SetGraphicsRootSignature(Renderer::Get()->m_BindlessRootSinature.Get());
		CommandList->GetD3D12CommandList()->SetPipelineState(CommonResources::Get()->m_SpriteHitProxyPSO->m_PSO);
		
		SetConstantAndSrv(CommandList, Renderer);
		
		CommonResources::Get()->m_UniformQuad->BindAndDraw(CommandList);
	}

	void BillboardSceneProxy::RenderSelectionPass( D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		
	}

	void BillboardSceneProxy::RenderEditorPrimitivePass( D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		CommandList->SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		CommandList->GetD3D12CommandList()->SetGraphicsRootSignature(Renderer::Get()->m_BindlessRootSinature.Get());
		CommandList->GetD3D12CommandList()->SetPipelineState(CommonResources::Get()->m_SpriteEditorPrimitivePSO->m_PSO);
		
		SetConstantAndSrv(CommandList, Renderer);
		
		CommonResources::Get()->m_UniformQuad->BindAndDraw(CommandList);
	}
#endif

	void BillboardSceneProxy::InitResources( D3D12CommandList* CommandList )
	{}

	void BillboardSceneProxy::UpdateResources( D3D12CommandList* CommandList )
	{
		
	}

	void BillboardSceneProxy::SetConstantAndSrv( D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		XMVECTOR Translation;
		XMVECTOR Scale;
		XMVECTOR Rotation;
		XMMatrixDecompose( &Scale, &Rotation, &Translation, m_LocalToWorld.Get() );
		
		ViewInfo VInfo = Renderer->GetScene()->GetWorld()->GetPlayerWorldView();
		Quat CameraRotation =  VInfo.Rotation;
		//Quat CameraRotation =  Renderer->GetScene()->GetWorld()->;

		Quat LocalRotation = Quat(0, 0, XM_PI);

		Matrix LocalToWorld = Transform(Translation, CameraRotation * LocalRotation);

		XMMATRIX mvpMatrix = XMMatrixMultiply( LocalToWorld.Get(), Renderer->GetSceneView().WorldToProjection.Get() );

		if (m_Sprite.IsValid())
		{
			if (m_Sprite->IsRenderStateDirty())
			{
				m_Sprite->UploadResources(CommandList);
			}

			// TODO: set default texture if null
			m_BillboardData.m_TextureIndex = m_Sprite.IsValid() ? m_Sprite->GetTextureIndex() : 0;
		}

		m_BillboardData.m_LocalToProjetcion = mvpMatrix;
		m_BillboardData.m_Guid = m_Guid;

		TRefCountPtr<RenderUniformBuffer> BillboardBuffer = RenderUniformBuffer::Create(CommandList->GetParentDevice(), sizeof(BillboardData), EUniformBufferUsage::SingleFrame, &m_BillboardData);

		CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, Renderer::Get()->GetBindlessSrvIndex(Renderer->m_BindlessViewBuffer[Renderer::Get()->GetCurrentBackbufferIndex()]->GetGpuHandle()), 0);
		CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, BillboardBuffer->GetViewIndex(), 1);
		CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, Renderer::Get()->GetBindlessSrvIndex(Renderer::Get()->m_StaticSamplersBuffer->GetGpuHandle()), 2);
	}

	PrimitiveComponent* BillboardSceneProxy::GetPrimitive()
	{
		return m_BillboardComponent;
	}

}