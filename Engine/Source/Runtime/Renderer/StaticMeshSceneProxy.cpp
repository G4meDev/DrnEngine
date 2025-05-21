#include "DrnPCH.h"
#include "StaticMeshSceneProxy.h"

LOG_DEFINE_CATEGORY( LogStaticMeshSceneProxy, "StaticMeshSceneProxy" );

namespace Drn
{
	StaticMeshSceneProxy::StaticMeshSceneProxy( StaticMeshComponent* InStaticMeshComponent )
		: PrimitiveSceneProxy( InStaticMeshComponent )
		, m_OwningStaticMeshComponent( InStaticMeshComponent )
		, m_Guid(InStaticMeshComponent->GetGuid())
	{
		m_EditorPrimitive = InStaticMeshComponent->IsEditorPrimitive();
#if WITH_EDITOR
		m_Selectable = InStaticMeshComponent->m_Selectable;
#endif
	}

	StaticMeshSceneProxy::~StaticMeshSceneProxy()
	{
	}

	void StaticMeshSceneProxy::InitResources( ID3D12GraphicsCommandList2* CommandList )
	{

	}

	void StaticMeshSceneProxy::UpdateResources( ID3D12GraphicsCommandList2* CommandList )
	{
		if (m_OwningStaticMeshComponent->IsRenderStateDirty())
		{
			m_Mesh = m_OwningStaticMeshComponent->GetMesh();
		}

		if (m_Mesh.IsValid())
		{
			m_Mesh->UploadResources(CommandList);
		}

		if (m_OwningStaticMeshComponent->IsRenderStateDirty())
		{
			m_Materials.clear();

			if (m_Mesh.IsValid())
			{
				const uint32 MaterialCount = m_Mesh->Data.Materials.size();
				const uint32 OverrideMaterialCount = m_OwningStaticMeshComponent->m_OverrideMaterials.size();
				m_Materials.resize(MaterialCount);

				for (int i = 0; i < MaterialCount; i++)
				{
					if (i < OverrideMaterialCount && m_OwningStaticMeshComponent->m_OverrideMaterials[i].m_Overriden)
					{
						m_Materials[i] = m_OwningStaticMeshComponent->m_OverrideMaterials[i].m_Material;
					}
					else
					{
						m_Materials[i] = m_Mesh->Data.Materials[i].m_Material;
					}

					// TODO: mark this only in with editor builds
					m_Materials[i].LoadChecked();
					if (!m_Materials[i].IsValid())
					{
						LOG(LogStaticMeshSceneProxy, Error, "Material is invalid. Using default material.");
						m_Materials[i] = AssetHandle<Material>(DEFAULT_MATERIAL_PATH);
						m_Materials[i].Load();
					}
				}
			}

		}

		for (AssetHandle<Material>& Mat : m_Materials)
		{
			Mat->UploadResources(CommandList);
		}

		m_OwningStaticMeshComponent->ClearRenderStateDirty();
	}

	void StaticMeshSceneProxy::RenderMainPass( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer )
	{
		if (m_Mesh.IsValid())
		{
			for (size_t i = 0; i < m_Mesh->Data.MeshesData.size(); i++)
			{
				const StaticMeshSlotData& RenderProxy = m_Mesh->Data.MeshesData[i];
				AssetHandle<Material>& Mat = m_Materials[RenderProxy.MaterialIndex];
				
				Mat->BindMainPass(CommandList);
				CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

				// TODO: remove dependency and only copy from parent side
				XMMATRIX modelMatrix = Matrix(m_OwningStaticMeshComponent->GetWorldTransform()).Get();

				float aspectRatio = (float) (Renderer->GetViewportSize().X) / Renderer->GetViewportSize().Y;
		
				XMMATRIX viewMatrix;
				XMMATRIX projectionMatrix;
		
				Renderer->m_CameraActor->GetCameraComponent()->CalculateMatrices(viewMatrix, projectionMatrix, aspectRatio);

				XMMATRIX mvpMatrix = XMMatrixMultiply( modelMatrix, viewMatrix );
				mvpMatrix          = XMMatrixMultiply( mvpMatrix, projectionMatrix );

				CommandList->SetGraphicsRoot32BitConstants( 0, 16, &mvpMatrix, 0);
				CommandList->SetGraphicsRoot32BitConstants( 0, 16, &modelMatrix, 16);
				CommandList->SetGraphicsRoot32BitConstants( 0, 4, &m_Guid, 32);

				CommandList->IASetVertexBuffers( 0, 1, &RenderProxy.m_VertexBufferView );
				CommandList->IASetIndexBuffer( &RenderProxy.m_IndexBufferView );
				CommandList->DrawIndexedInstanced( RenderProxy.m_IndexBufferView.SizeInBytes / sizeof(uint32), 1, 0, 0, 0);
			}
		}
	}

#if WITH_EDITOR

	void StaticMeshSceneProxy::RenderHitProxyPass( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer )
	{
		if (!m_Selectable)
		{
			return;
		}

		for (size_t i = 0; i < m_Mesh->Data.MeshesData.size(); i++)
		{
			const StaticMeshSlotData& RenderProxy = m_Mesh->Data.MeshesData[i];
			AssetHandle<Material>& Mat = m_Materials[RenderProxy.MaterialIndex];

			if (!Mat->IsSupportingHitProxyPass())
			{
				continue;
			}

			Mat->BindHitProxyPass(CommandList);
			CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			// TODO: remove dependency and only copy from parent side
			XMMATRIX modelMatrix = Matrix(m_OwningStaticMeshComponent->GetWorldTransform()).Get();

			float aspectRatio = (float) (Renderer->GetViewportSize().X) / Renderer->GetViewportSize().Y;
		
			XMMATRIX viewMatrix;
			XMMATRIX projectionMatrix;
		
			Renderer->m_CameraActor->GetCameraComponent()->CalculateMatrices(viewMatrix, projectionMatrix, aspectRatio);

			XMMATRIX mvpMatrix = XMMatrixMultiply( modelMatrix, viewMatrix );
			mvpMatrix          = XMMatrixMultiply( mvpMatrix, projectionMatrix );

			CommandList->SetGraphicsRoot32BitConstants( 0, 16, &mvpMatrix, 0);
			CommandList->SetGraphicsRoot32BitConstants( 0, 16, &modelMatrix, 16);
			CommandList->SetGraphicsRoot32BitConstants( 0, 4, &m_Guid, 32);

			CommandList->IASetVertexBuffers( 0, 1, &RenderProxy.m_VertexBufferView );
			CommandList->IASetIndexBuffer( &RenderProxy.m_IndexBufferView );
			CommandList->DrawIndexedInstanced( RenderProxy.m_IndexBufferView.SizeInBytes / sizeof(uint32), 1, 0, 0, 0);
		}
	}

	void StaticMeshSceneProxy::RenderSelectionPass( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer )
	{
		if (!m_SelectedInEditor)
			return;

		for (size_t i = 0; i < m_Mesh->Data.MeshesData.size(); i++)
		{
			const StaticMeshSlotData& RenderProxy = m_Mesh->Data.MeshesData[i];
			AssetHandle<Material>& Mat = m_Materials[RenderProxy.MaterialIndex];
				
			Mat->BindSelectionPass(CommandList);
			CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			// TODO: remove dependency and only copy from parent side
			XMMATRIX modelMatrix = Matrix(m_OwningStaticMeshComponent->GetWorldTransform()).Get();

			float aspectRatio = (float) (Renderer->GetViewportSize().X) / Renderer->GetViewportSize().Y;
		
			XMMATRIX viewMatrix;
			XMMATRIX projectionMatrix;
		
			Renderer->m_CameraActor->GetCameraComponent()->CalculateMatrices(viewMatrix, projectionMatrix, aspectRatio);

			XMMATRIX mvpMatrix = XMMatrixMultiply( modelMatrix, viewMatrix );
			mvpMatrix          = XMMatrixMultiply( mvpMatrix, projectionMatrix );

			CommandList->SetGraphicsRoot32BitConstants( 0, 16, &mvpMatrix, 0);
			CommandList->SetGraphicsRoot32BitConstants( 0, 16, &modelMatrix, 16);
			CommandList->SetGraphicsRoot32BitConstants( 0, 4, &m_Guid, 32);

			CommandList->IASetVertexBuffers( 0, 1, &RenderProxy.m_VertexBufferView );
			CommandList->IASetIndexBuffer( &RenderProxy.m_IndexBufferView );
			CommandList->DrawIndexedInstanced( RenderProxy.m_IndexBufferView.SizeInBytes / sizeof(uint32), 1, 0, 0, 0);
		}
	}

	void StaticMeshSceneProxy::RenderEditorPrimitivePass( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer )
	{
		if (m_Mesh.IsValid())
		{
			for (size_t i = 0; i < m_Mesh->Data.MeshesData.size(); i++)
			{
				const StaticMeshSlotData& RenderProxy = m_Mesh->Data.MeshesData[i];
				AssetHandle<Material>& Mat = m_Materials[RenderProxy.MaterialIndex];
				
				Mat->BindEditorPrimitivePass(CommandList);
				CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

				// TODO: remove dependency and only copy from parent side
				XMMATRIX modelMatrix = Matrix(m_OwningStaticMeshComponent->GetWorldTransform()).Get();

				float aspectRatio = (float) (Renderer->GetViewportSize().X) / Renderer->GetViewportSize().Y;
		
				XMMATRIX viewMatrix;
				XMMATRIX projectionMatrix;
		
				Renderer->m_CameraActor->GetCameraComponent()->CalculateMatrices(viewMatrix, projectionMatrix, aspectRatio);

				XMMATRIX mvpMatrix = XMMatrixMultiply( modelMatrix, viewMatrix );
				mvpMatrix          = XMMatrixMultiply( mvpMatrix, projectionMatrix );

				CommandList->SetGraphicsRoot32BitConstants( 0, 16, &mvpMatrix, 0);
				CommandList->SetGraphicsRoot32BitConstants( 0, 16, &modelMatrix, 16);
				CommandList->SetGraphicsRoot32BitConstants( 0, 4, &m_Guid, 32);

				CommandList->IASetVertexBuffers( 0, 1, &RenderProxy.m_VertexBufferView );
				CommandList->IASetIndexBuffer( &RenderProxy.m_IndexBufferView );
				CommandList->DrawIndexedInstanced( RenderProxy.m_IndexBufferView.SizeInBytes / sizeof(uint32), 1, 0, 0, 0);
			}
		}
	}

#endif

}