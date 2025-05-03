#include "DrnPCH.h"
#include "StaticMeshSceneProxy.h"

LOG_DEFINE_CATEGORY( LogStaticMeshSceneProxy, "StaticMeshSceneProxy" );

#define DEFAULT_MATERIAL_PATH "\\Engine\\Content\\Materials\\M_DefaultMeshShader.drn"

namespace Drn
{
	StaticMeshSceneProxy::StaticMeshSceneProxy( StaticMeshComponent* InStaticMeshComponent )
		: PrimitiveSceneProxy( InStaticMeshComponent )
		, m_OwningStaticMeshComponent( InStaticMeshComponent )
	{
	}

	StaticMeshSceneProxy::~StaticMeshSceneProxy()
	{
	}

	void StaticMeshSceneProxy::InitResources( dx12lib::CommandList* CommandList )
	{

	}

	void StaticMeshSceneProxy::UpdateResources( dx12lib::CommandList* CommandList )
	{
		StaticMesh* Mesh =  m_OwningStaticMeshComponent->GetMesh();

		if (Mesh)
		{
			if (!Mesh->IsLoadedOnGpu())
			{
				Mesh->UploadResources(CommandList);
				m_OwningStaticMeshComponent->MarkRenderStateDirty();
			}
		}

		if (m_OwningStaticMeshComponent->IsRenderStateDirty())
		{
			m_Materials.clear();

			if (Mesh)
			{
				const uint32 MaterialCount = Mesh->Data.Materials.size();
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
						m_Materials[i] = Mesh->Data.Materials[i].m_Material;
					}

					// TODO: mark this only in with editor builds
					m_Materials[i].LoadChecked();
					if (!m_Materials[i].IsValid())
					{
						LOG(LogStaticMeshSceneProxy, Error, "Material is invalid. Using default material.");
						m_Materials[i] = AssetHandle<Material>(DEFAULT_MATERIAL_PATH);
						m_Materials[i].Load();
					}

					if (!m_Materials[i]->IsLoadedOnGpu())
					{
						m_Materials[i]->UploadResources(CommandList);
					}
				}
			}

			m_OwningStaticMeshComponent->ClearRenderStateDirty();
		}
	}

	void StaticMeshSceneProxy::RenderMainPass( dx12lib::CommandList* CommandList, SceneRenderer* Renderer )
	{
		StaticMesh* Mesh =  m_OwningStaticMeshComponent->GetMesh();

		if (Mesh)
		{
			for (size_t i = 0; i < Mesh->Data.MeshesData.size(); i++)
			{
				const StaticMeshSlotData& RenderProxy = Mesh->Data.MeshesData[i];
				//MaterialData& Mat = Mesh->Data.Materials[RenderProxy.MaterialIndex];
				AssetHandle<Material>& Mat = m_Materials[RenderProxy.MaterialIndex];

				CommandList->GetD3D12CommandList()->SetGraphicsRootSignature(Mat->GetRootSignature());
				CommandList->GetD3D12CommandList()->SetPipelineState(Mat->GetBasePassPSO());
				CommandList->GetD3D12CommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

				XMMATRIX modelMatrix = Matrix(m_OwningStaticMeshComponent->GetWorldTransform()).Get();

				auto viewport = Renderer->m_RenderTarget.GetViewport();
				float    aspectRatio = viewport.Width / viewport.Height;
		
				XMMATRIX viewMatrix;
				XMMATRIX projectionMatrix;
		
				Renderer->m_CameraActor->GetCameraComponent()->CalculateMatrices(viewMatrix, projectionMatrix, aspectRatio);

				XMMATRIX mvpMatrix = XMMatrixMultiply( modelMatrix, viewMatrix );
				mvpMatrix          = XMMatrixMultiply( mvpMatrix, projectionMatrix );

				CommandList->SetGraphics32BitConstants( 0, mvpMatrix );

				CommandList->SetVertexBuffer( 0, RenderProxy.VertexBuffer );
				CommandList->SetIndexBuffer( RenderProxy.IndexBuffer );
				CommandList->DrawIndexed( RenderProxy.IndexBuffer->GetNumIndices() );
			}
		}
	}
}