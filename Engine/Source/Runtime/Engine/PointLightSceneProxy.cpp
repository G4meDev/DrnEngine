#include "DrnPCH.h"
#include "PointLightSceneProxy.h"
#include "Runtime/Components/PointLightComponent.h"

namespace Drn
{
	PointLightSceneProxy::PointLightSceneProxy( class PointLightComponent* InComponent )
		: LightSceneProxy(InComponent)
		, m_Radius(InComponent->GetRadius())
	{
	}

	PointLightSceneProxy::~PointLightSceneProxy()
	{
	}

	void PointLightSceneProxy::Render( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer )
	{
		float aspectRatio = (float) (Renderer->GetViewportSize().X) / Renderer->GetViewportSize().Y;

		XMMATRIX viewMatrix;
		XMMATRIX projectionMatrix;
		
		Renderer->m_CameraActor->GetCameraComponent()->CalculateMatrices(viewMatrix, projectionMatrix, aspectRatio);

		XMMATRIX LocalToWorld = XMMatrixTranslationFromVector( XMLoadFloat3( m_WorldPosition.Get() ) );

		XMMATRIX mvpMatrix = XMMatrixMultiply( LocalToWorld, viewMatrix );
		mvpMatrix          = XMMatrixMultiply( mvpMatrix, projectionMatrix );

		CommandList->SetGraphicsRoot32BitConstants( 0, 16, &mvpMatrix, 0);


		//if (m_Sprite.IsValid())
		//{
		//	if (m_Sprite->IsRenderStateDirty())
		//	{
		//		m_Sprite->UploadResources(CommandList);
		//	}
		//
		//	CommandList->SetGraphicsRootDescriptorTable( 1, m_Sprite->TextureGpuHandle );
		//}

		CommandList->IASetVertexBuffers( 0, 1, &CommonResources::Get()->m_PointLightSphere->m_VertexBufferView);
		CommandList->IASetIndexBuffer( &CommonResources::Get()->m_PointLightSphere->m_IndexBufferView );
		CommandList->DrawIndexedInstanced( CommonResources::Get()->m_PointLightSphere->m_IndexCount, 1, 0, 0, 0);
	}

#if WITH_EDITOR
	void PointLightSceneProxy::DrawAttenuation(World* InWorld)
	{
		InWorld->DrawDebugSphere( m_WorldPosition, Quat::Identity, Vector::OneVector, m_Radius, 12, 5, 0 );
	}
#endif
}