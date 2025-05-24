#include "DrnPCH.h"
#include "PointLightSceneProxy.h"
#include "Runtime/Components/PointLightComponent.h"

namespace Drn
{
	PointLightSceneProxy::PointLightSceneProxy( class PointLightComponent* InComponent )
		: LightSceneProxy(InComponent)
		, m_Radius(InComponent->GetRadius())
	{
		SetLocalToWorld(InComponent->GetLocalToWorld());
	}

	PointLightSceneProxy::~PointLightSceneProxy()
	{
	}

	void PointLightSceneProxy::Render( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer )
	{
		float aspectRatio = (float) (Renderer->GetViewportSize().X) / Renderer->GetViewportSize().Y;

		XMMATRIX viewMatrix;
		XMMATRIX projectionMatrix;
		
		Vector CameraPosition = Renderer->m_CameraActor->GetActorLocation();

		Renderer->m_CameraActor->GetCameraComponent()->CalculateMatrices(viewMatrix, projectionMatrix, aspectRatio);
		XMMATRIX LocalToWorld = XMMatrixTranslationFromVector( XMLoadFloat3( m_WorldPosition.Get() ) );

		XMMATRIX mvpMatrix = XMMatrixMultiply( m_LocalToWorld, viewMatrix );
		mvpMatrix          = XMMatrixMultiply( mvpMatrix, projectionMatrix );

		CommandList->SetGraphicsRoot32BitConstants( 0, 16, &mvpMatrix, 0);
		CommandList->SetGraphicsRoot32BitConstants( 0, 16, &XMMatrixInverse(NULL, XMMatrixMultiply(viewMatrix, projectionMatrix)), 16);
		CommandList->SetGraphicsRoot32BitConstants( 0, 4, &Vector4(CameraPosition, 1), 32);
		CommandList->SetGraphicsRoot32BitConstants( 0, 4, &Vector4(m_WorldPosition, m_Radius), 36);
		CommandList->SetGraphicsRoot32BitConstants( 0, 4, &Vector4(m_LightColor, 1), 40);


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
		InWorld->DrawDebugSphere( m_WorldPosition, Quat::Identity, Color::White, m_Radius, 36, 0.0, 0 );
	}
#endif
}