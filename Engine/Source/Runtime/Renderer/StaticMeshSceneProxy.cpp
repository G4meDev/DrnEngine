#include "DrnPCH.h"
#include "StaticMeshSceneProxy.h"

namespace Drn
{
	StaticMeshSceneProxy::StaticMeshSceneProxy( StaticMeshComponent* InStaticMeshComponent )
		: PrimitiveSceneProxy( InStaticMeshComponent )
		, m_OwningStaticMeshComponent( InStaticMeshComponent )
	{
		m_TempMat = AssetHandle<Material>("Engine\\Content\\Materials\\M_TestShader.drn");
		m_TempMat.Load();
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
			}
		}

		if (m_TempMat.IsValid() && !m_TempMat->IsLoadedOnGpu())
		{
			m_TempMat->UploadResources(CommandList);
		}
	}

	void StaticMeshSceneProxy::RenderMainPass( dx12lib::CommandList* CommandList, SceneRenderer* Renderer )
	{
		StaticMesh* Mesh =  m_OwningStaticMeshComponent->GetMesh();

		if (Mesh && m_TempMat.IsValid())
		{
			CommandList->GetD3D12CommandList()->SetGraphicsRootSignature(m_TempMat->GetRootSignature());
			CommandList->GetD3D12CommandList()->SetPipelineState(m_TempMat->GetBasePassPSO());
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

			for (const StaticMeshSlotData& RenderProxy : Mesh->Data.MeshesData)
			{
				CommandList->SetVertexBuffer( 0, RenderProxy.VertexBuffer );
				CommandList->SetIndexBuffer( RenderProxy.IndexBuffer );
				CommandList->DrawIndexed( RenderProxy.IndexBuffer->GetNumIndices() );
			}
		}
	}
}