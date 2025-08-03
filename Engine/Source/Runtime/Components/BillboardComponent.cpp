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
		, m_BillboardBuffer(nullptr)
	{
	}

	BillboardSceneProxy::~BillboardSceneProxy()
	{
		if (m_BillboardBuffer)
		{
			m_BillboardBuffer->ReleaseBufferedResource();
			m_BillboardBuffer = nullptr;
		}
	}

	void BillboardSceneProxy::RenderMainPass( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer )
	{
		
	}

	void BillboardSceneProxy::RenderShadowPass( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer, LightSceneProxy* LightProxy )
	{
		
	}

#if WITH_EDITOR
	void BillboardSceneProxy::RenderHitProxyPass( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer )
	{
		CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		CommandList->SetGraphicsRootSignature(Renderer::Get()->m_BindlessRootSinature.Get());
		CommandList->SetPipelineState(CommonResources::Get()->m_SpriteHitProxyPSO->m_PSO);
		
		SetConstantAndSrv(CommandList, Renderer);
		
		CommonResources::Get()->m_UniformQuad->BindAndDraw(CommandList);
	}

	void BillboardSceneProxy::RenderSelectionPass( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer )
	{
		
	}

	void BillboardSceneProxy::RenderEditorPrimitivePass( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer )
	{
		CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		CommandList->SetGraphicsRootSignature(Renderer::Get()->m_BindlessRootSinature.Get());
		CommandList->SetPipelineState(CommonResources::Get()->m_SpriteEditorPrimitivePSO->m_PSO);
		
		SetConstantAndSrv(CommandList, Renderer);
		
		CommonResources::Get()->m_UniformQuad->BindAndDraw(CommandList);
	}
#endif

	void BillboardSceneProxy::InitResources( ID3D12GraphicsCommandList2* CommandList )
	{
		m_BillboardBuffer = Resource::Create(D3D12_HEAP_TYPE_UPLOAD, CD3DX12_RESOURCE_DESC::Buffer( 256 ), D3D12_RESOURCE_STATE_GENERIC_READ);

		D3D12_CONSTANT_BUFFER_VIEW_DESC ResourceViewDesc = {};
		ResourceViewDesc.BufferLocation = m_BillboardBuffer->GetD3D12Resource()->GetGPUVirtualAddress();
		ResourceViewDesc.SizeInBytes = 256;
		Renderer::Get()->GetD3D12Device()->CreateConstantBufferView( &ResourceViewDesc, m_BillboardBuffer->GetCpuHandle());

#if D3D12_Debug_INFO
		m_BillboardBuffer->SetName("ConstantBufferBillboard_" + m_Name);
#endif
	}

	void BillboardSceneProxy::UpdateResources( ID3D12GraphicsCommandList2* CommandList )
	{
		
	}

	void BillboardSceneProxy::SetConstantAndSrv( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer )
	{
		XMVECTOR Translation;
		XMVECTOR Scale;
		XMVECTOR Rotation;
		XMMatrixDecompose( &Scale, &Rotation, &Translation, m_LocalToWorld.Get() );
		
		Quat CameraRotation =  Renderer->m_CameraActor->GetActorRotation();
		Quat LocalRotation = Quat(0, 0, XM_PI);

		Matrix LocalToWorld = Transform(Translation, CameraRotation * LocalRotation);

		XMMATRIX mvpMatrix = XMMatrixMultiply( LocalToWorld.Get(), Renderer->GetSceneView().WorldToProjection.Get() );

		if (m_Sprite.IsValid())
		{
			if (m_Sprite->IsRenderStateDirty())
			{
				m_Sprite->UploadResources(CommandList);
			}

			if (m_Sprite->GetResource())
			{
				// TODO: set default texture if null
				m_BillboardData.m_TextureIndex = m_Sprite->GetResource() ? Renderer::Get()->GetBindlessSrvIndex(m_Sprite->GetResource()->GetGpuHandle()) : 0;
			}
		}

		m_BillboardData.m_LocalToProjetcion = mvpMatrix;
		m_BillboardData.m_Guid = m_Guid;

		UINT8* ConstantBufferStart;
		CD3DX12_RANGE readRange( 0, 0 );
		m_BillboardBuffer->GetD3D12Resource()->Map(0, &readRange, reinterpret_cast<void**>( &ConstantBufferStart ) );
		memcpy( ConstantBufferStart, &m_BillboardData, sizeof(BillboardData));
		m_BillboardBuffer->GetD3D12Resource()->Unmap(0, nullptr);

		CommandList->SetGraphicsRoot32BitConstant(0, Renderer::Get()->GetBindlessSrvIndex(Renderer->m_BindlessViewBuffer->GetGpuHandle()), 0);
		CommandList->SetGraphicsRoot32BitConstant(0, Renderer::Get()->GetBindlessSrvIndex(m_BillboardBuffer->GetGpuHandle()), 1);
		CommandList->SetGraphicsRoot32BitConstant(0, Renderer::Get()->GetBindlessSrvIndex(Renderer::Get()->m_StaticSamplersBuffer->GetGpuHandle()), 2);
	}

	PrimitiveComponent* BillboardSceneProxy::GetPrimitive()
	{
		return m_BillboardComponent;
	}

}