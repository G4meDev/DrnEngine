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
	{
	}

	BillboardSceneProxy::~BillboardSceneProxy()
	{
		
	}

	void BillboardSceneProxy::RenderMainPass( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer )
	{
		
	}

	void BillboardSceneProxy::RenderHitProxyPass( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer )
	{
		CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		CommandList->SetPipelineState(CommonResources::Get()->m_SpriteHitProxyPSO->m_PSO);
		CommandList->SetGraphicsRootSignature(CommonResources::Get()->m_SpriteHitProxyPSO->m_RootSignature);

		SetConstantAndSrv(CommandList, Renderer);

		CommonResources::Get()->m_UniformQuad->BindAndDraw(CommandList);
	}

	void BillboardSceneProxy::RenderSelectionPass( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer )
	{
		
	}

	void BillboardSceneProxy::RenderEditorPrimitivePass( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer )
	{
		CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		CommandList->SetPipelineState(CommonResources::Get()->m_SpriteEditorPrimitivePSO->m_PSO);
		CommandList->SetGraphicsRootSignature(CommonResources::Get()->m_SpriteEditorPrimitivePSO->m_RootSignature);

		SetConstantAndSrv(CommandList, Renderer);

		CommonResources::Get()->m_UniformQuad->BindAndDraw(CommandList);
	}

	void BillboardSceneProxy::InitResources( ID3D12GraphicsCommandList2* CommandList )
	{
		
	}

	void BillboardSceneProxy::UpdateResources( ID3D12GraphicsCommandList2* CommandList )
	{
		
	}

	void BillboardSceneProxy::SetConstantAndSrv( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer )
	{
		float aspectRatio = (float) (Renderer->GetViewportSize().X) / Renderer->GetViewportSize().Y;
		
		XMMATRIX viewMatrix;
		XMMATRIX projectionMatrix;
		
		Renderer->m_CameraActor->GetCameraComponent()->CalculateMatrices(viewMatrix, projectionMatrix, aspectRatio);

		XMVECTOR Translation;
		XMVECTOR Scale;
		XMVECTOR Rotation;
		XMMatrixDecompose( &Scale, &Rotation, &Translation, m_LocalToWorld.Get() );

		XMVECTOR CameraLocation =  XMLoadFloat3( Renderer->m_CameraActor->GetActorLocation().Get() );

		XMVECTOR Direction = CameraLocation - Translation;
		XMMATRIX LookAtCamera = Matrix::MakeFromZ( Vector(Direction) ).Get();
		XMMATRIX CameraRotated = XMMatrixMultiply( LookAtCamera, m_LocalToWorld.Get() );

		XMMATRIX mvpMatrix = XMMatrixMultiply( CameraRotated, viewMatrix );
		mvpMatrix          = XMMatrixMultiply( mvpMatrix, projectionMatrix );

		CommandList->SetGraphicsRoot32BitConstants( 0, 16, &mvpMatrix, 0);
		CommandList->SetGraphicsRoot32BitConstants( 0, 4, &m_Guid, 16);

		if (m_Sprite.IsValid())
		{
			if (m_Sprite->IsRenderStateDirty())
			{
				m_Sprite->UploadResources(CommandList);
			}

			CommandList->SetGraphicsRootDescriptorTable( 1, m_Sprite->TextureGpuHandle );
		}

		else
		{
			// TODO: set default texture if null
		}
	}

	PrimitiveComponent* BillboardSceneProxy::GetPrimitive()
	{
		return m_BillboardComponent;
	}

}