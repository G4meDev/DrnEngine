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
		Vector CameraPosition = Renderer->m_CameraActor->GetActorLocation();
		XMMATRIX LocalToWorld = XMMatrixTranslationFromVector( XMLoadFloat3( m_WorldPosition.Get() ) );
		XMMATRIX LocalToProjection = XMMatrixMultiply( m_LocalToWorld, Renderer->GetSceneView().WorldToProjection.Get() );

		Vector4 Term1(CameraPosition, 1);
		Vector4 Term2(m_WorldPosition, m_Radius);
		Vector4 Term3(m_LightColor, 1);

		CommandList->SetGraphicsRoot32BitConstants( 0, 16, &LocalToProjection, 0);
		CommandList->SetGraphicsRoot32BitConstants( 0, 16, &Renderer->GetSceneView().ProjectionToWorld, 16);
		CommandList->SetGraphicsRoot32BitConstants( 0, 4, &Term1, 32);
		CommandList->SetGraphicsRoot32BitConstants( 0, 4, &Term2, 36);
		CommandList->SetGraphicsRoot32BitConstants( 0, 4, &Term3, 40);


		//if (m_Sprite.IsValid())
		//{
		//	if (m_Sprite->IsRenderStateDirty())
		//	{
		//		m_Sprite->UploadResources(CommandList);
		//	}
		//
		//	CommandList->SetGraphicsRootDescriptorTable( 1, m_Sprite->TextureGpuHandle );
		//}

		CommonResources::Get()->m_PointLightSphere->BindAndDraw(CommandList);
	}

#if WITH_EDITOR
	void PointLightSceneProxy::DrawAttenuation(World* InWorld)
	{
		InWorld->DrawDebugSphere( m_WorldPosition, Quat::Identity, Color::White, m_Radius, 36, 0.0, 0 );
	}
#endif
}