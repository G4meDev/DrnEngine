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
		if (m_PrimitiveSource)
		{
			m_PrimitiveSource->ReleaseBufferedResource();
			m_PrimitiveSource = nullptr;
		}

		Renderer::Get()->m_BindlessSrvHeapAllocator.Free(m_BufferCpuHandle, m_BufferGpuHandle);
	}

	void StaticMeshSceneProxy::InitResources( ID3D12GraphicsCommandList2* CommandList )
	{
		if (m_PrimitiveSource)
		{
			return;
		}

		Renderer::Get()->m_BindlessSrvHeapAllocator.Alloc(&m_BufferCpuHandle, &m_BufferGpuHandle);
		m_PrimitiveSource = Resource::Create(D3D12_HEAP_TYPE_UPLOAD, CD3DX12_RESOURCE_DESC::Buffer( 256 ), D3D12_RESOURCE_STATE_GENERIC_READ);

		D3D12_CONSTANT_BUFFER_VIEW_DESC ResourceViewDesc = {};
		ResourceViewDesc.BufferLocation = m_PrimitiveSource->GetD3D12Resource()->GetGPUVirtualAddress();
		ResourceViewDesc.SizeInBytes = 256;
		Renderer::Get()->GetD3D12Device()->CreateConstantBufferView( &ResourceViewDesc, m_BufferCpuHandle);

#if WITH_EDITOR
		m_PrimitiveSource->SetName("Primitive Buffer 123");
#endif
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
				
				if (!Mat->IsSupportingMainPass())
				{
					continue;
				}

				if (!Mat->m_BindlessTest)
				{
					Mat->BindMainPass(CommandList);
					CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

					// TODO: remove dependency and only copy from parent side
					XMMATRIX LocalToWorld = Matrix(m_OwningStaticMeshComponent->GetWorldTransform()).Get();
					XMMATRIX LocalToProjection = XMMatrixMultiply( LocalToWorld, Renderer->GetSceneView().WorldToProjection.Get() );

					CommandList->SetGraphicsRoot32BitConstants( 0, 16, &LocalToProjection, 0);
					CommandList->SetGraphicsRoot32BitConstants( 0, 16, &LocalToWorld, 16);
					CommandList->SetGraphicsRoot32BitConstants( 0, 4, &m_Guid, 32);

					RenderProxy.BindAndDraw(CommandList);
				}

				else
				{
					Renderer::Get()->SetBindlessHeaps(CommandList);

					CommandList->SetGraphicsRootSignature( Renderer::Get()->m_BindlessRootSinature.Get() );
					CommandList->SetPipelineState( Mat->m_MainPassPSO->GetD3D12PSO() );

					CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

					m_PrimitiveBuffer.m_LocalToWorld = Matrix(m_OwningStaticMeshComponent->GetWorldTransform()).Get();
					m_PrimitiveBuffer.m_LocalToProjection = XMMatrixMultiply( m_PrimitiveBuffer.m_LocalToWorld.Get(), Renderer->GetSceneView().WorldToProjection.Get() );
					m_PrimitiveBuffer.m_Guid = m_Guid;

					UINT8* ConstantBufferStart;
					CD3DX12_RANGE readRange( 0, 0 );
					m_PrimitiveSource->GetD3D12Resource()->Map(0, &readRange, reinterpret_cast<void**>( &ConstantBufferStart ) );
					memcpy( ConstantBufferStart, &m_PrimitiveBuffer, sizeof(PrimitiveBuffer));
					m_PrimitiveSource->GetD3D12Resource()->Unmap(0, nullptr);

					CommandList->SetGraphicsRoot32BitConstant(0, Renderer::Get()->GetBindlessSrvIndex(Renderer->m_BindlessViewGpuHandle), 0);
					CommandList->SetGraphicsRoot32BitConstant(0, Renderer::Get()->GetBindlessSrvIndex(m_BufferGpuHandle), 1);

					RenderProxy.BindAndDraw(CommandList);
				}

			}
		}
	}

	void StaticMeshSceneProxy::RenderShadowPass( ID3D12GraphicsCommandList2* CommandList, LightSceneProxy* LightProxy )
	{
		if (m_Mesh.IsValid())
		{
			for (size_t i = 0; i < m_Mesh->Data.MeshesData.size(); i++)
			{
				const StaticMeshSlotData& RenderProxy = m_Mesh->Data.MeshesData[i];
				AssetHandle<Material>& Mat = m_Materials[RenderProxy.MaterialIndex];

				if (!Mat->IsSupportingShadowPass())
				{
					continue;
				}

				// TODO: make dependent on light type
				Mat->BindPointLightShadowDepthPass(CommandList);
				CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		
				// TODO: remove dependency and only copy from parent side
				XMMATRIX LocalToWorld = Matrix(m_OwningStaticMeshComponent->GetWorldTransform()).Get();
				//XMMATRIX LocalToProjection = XMMatrixMultiply( LocalToWorld, Renderer->GetSceneView().WorldToProjection.Get() );
		
				//CommandList->SetGraphicsRoot32BitConstants( 0, 16, &LocalToProjection, 0);
				CommandList->SetGraphicsRoot32BitConstants( 0, 16, &LocalToWorld, 16);
				CommandList->SetGraphicsRoot32BitConstants( 0, 4, &m_Guid, 32);
		
				RenderProxy.BindAndDraw(CommandList);
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
			XMMATRIX LocalToWorld = Matrix(m_OwningStaticMeshComponent->GetWorldTransform()).Get();
			XMMATRIX LocalToProjection = XMMatrixMultiply( LocalToWorld, Renderer->GetSceneView().WorldToProjection.Get() );

			CommandList->SetGraphicsRoot32BitConstants( 0, 16, &LocalToProjection, 0);
			CommandList->SetGraphicsRoot32BitConstants( 0, 16, &LocalToWorld, 16);
			CommandList->SetGraphicsRoot32BitConstants( 0, 4, &m_Guid, 32);

			RenderProxy.BindAndDraw( CommandList );
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

			if (!Mat->IsSupportingEditorSelectionPass())
			{
				continue;
			}

			Mat->BindSelectionPass(CommandList);
			CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			// TODO: remove dependency and only copy from parent side
			XMMATRIX LocalToWorld = Matrix(m_OwningStaticMeshComponent->GetWorldTransform()).Get();
			XMMATRIX LocalToProjection = XMMatrixMultiply( LocalToWorld, Renderer->GetSceneView().WorldToProjection.Get() );

			CommandList->SetGraphicsRoot32BitConstants( 0, 16, &LocalToProjection, 0);
			CommandList->SetGraphicsRoot32BitConstants( 0, 16, &LocalToWorld, 16);
			CommandList->SetGraphicsRoot32BitConstants( 0, 4, &m_Guid, 32);

			RenderProxy.BindAndDraw(CommandList);
		}
	}

	void StaticMeshSceneProxy::RenderEditorPrimitivePass( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer )
	{
		if (!m_EditorPrimitive)
		{
			return;
		}

		if (m_Mesh.IsValid())
		{
			for (size_t i = 0; i < m_Mesh->Data.MeshesData.size(); i++)
			{
				const StaticMeshSlotData& RenderProxy = m_Mesh->Data.MeshesData[i];
				AssetHandle<Material>& Mat = m_Materials[RenderProxy.MaterialIndex];
				
				if (!Mat->IsSupportingEditorPrimitivePass())
				{
					continue;
				}

				Mat->BindEditorPrimitivePass(CommandList);
				CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

				// TODO: remove dependency and only copy from parent side
				XMMATRIX LocalToWorld = Matrix(m_OwningStaticMeshComponent->GetWorldTransform()).Get();
				XMMATRIX LocalToProjection = XMMatrixMultiply( LocalToWorld, Renderer->GetSceneView().WorldToProjection.Get() );

				CommandList->SetGraphicsRoot32BitConstants( 0, 16, &LocalToProjection, 0);
				CommandList->SetGraphicsRoot32BitConstants( 0, 16, &LocalToWorld, 16);
				CommandList->SetGraphicsRoot32BitConstants( 0, 4, &m_Guid, 32);

				RenderProxy.BindAndDraw( CommandList );
			}
		}
	}

#endif

}