#include "DrnPCH.h"
#include "InstancedStaticMeshSceneProxy.h"

namespace Drn
{
	InstancedStaticMeshSceneProxy::InstancedStaticMeshSceneProxy( InstancedStaticMeshComponent* InInstancedStaticMeshComponent )
		: PrimitiveSceneProxy(InInstancedStaticMeshComponent)
		, m_OwningInstancedStaticMeshComponent(InInstancedStaticMeshComponent)
		, m_Guid(InInstancedStaticMeshComponent->GetGuid())
	{
#if WITH_EDITOR
		m_EditorPrimitive = InInstancedStaticMeshComponent->IsEditorPrimitive();
		m_Selectable = InInstancedStaticMeshComponent->m_Selectable;
#endif

		MinDrawDistance = InInstancedStaticMeshComponent->MinDrawDistance;
		MaxDrawDistance = InInstancedStaticMeshComponent->MaxDrawDistance;
	}

	InstancedStaticMeshSceneProxy::~InstancedStaticMeshSceneProxy()
	{}

	const BoxSphereBounds& InstancedStaticMeshSceneProxy::GetBounds()
	{
		// TODO: only update bounds when dirty
		if (m_OwningInstancedStaticMeshComponent)
		{
			Bounds = m_OwningInstancedStaticMeshComponent->GetBounds();
		}

		return Bounds;
	}

	void InstancedStaticMeshSceneProxy::RenderMainPass( class D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		if (m_Mesh.IsValid())
		{
			const std::string MeshName = Path::GetCleanName(m_Mesh.GetPath());
			SCOPE_STAT_DYNAMIC(MeshName.c_str());

			for (size_t i = 0; i < m_Mesh->Data.MeshesData.size(); i++)
			{
				const StaticMeshSlotData& RenderProxy = m_Mesh->Data.MeshesData[i];
				MaterialSlot& Mat = m_Materials[RenderProxy.MaterialIndex];
				
				if (!Mat.GetParentMaterial()->IsSupportingBasePass())
				{
					continue;
				}

				SCOPE_STAT_DYNAMIC(Mat.GetMaterialName().c_str());

				Mat.GetParentMaterial()->BindMainPass( CommandList );
				Mat.GetMaterialInterface()->BindResources(CommandList);

				m_PrimitiveBuffer.m_LocalToWorld = Matrix(m_OwningInstancedStaticMeshComponent->GetWorldTransform()).Get();
				m_PrimitiveBuffer.m_LocalToProjection = XMMatrixMultiply( m_PrimitiveBuffer.m_LocalToWorld.Get(), Renderer->GetSceneView().WorldToProjection.Get() );
				m_PrimitiveBuffer.m_Guid = m_Guid;

				// TODO: cache in different draws
				TRefCountPtr<RenderUniformBuffer> MeshBuffer = RenderUniformBuffer::Create(CommandList->GetParentDevice(), sizeof(PrimitiveBuffer), EUniformBufferUsage::SingleFrame, &m_PrimitiveBuffer);

				CommandList->SetGraphicRootConstant(Renderer->ViewBuffer->GetViewIndex(), 0);
				CommandList->SetGraphicRootConstant(MeshBuffer->GetViewIndex(), 1);
				CommandList->SetGraphicRootConstant(Renderer::Get()->StaticSamplersBuffer->GetViewIndex(), 2);

				RenderProxy.BindAndDraw(CommandList);
			}
		}
	}

	void InstancedStaticMeshSceneProxy::RenderPrePass( class D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		if (m_Mesh.IsValid())
		{
			const std::string MeshName = Path::GetCleanName(m_Mesh.GetPath());
			SCOPE_STAT_DYNAMIC(MeshName.c_str());

			for (size_t i = 0; i < m_Mesh->Data.MeshesData.size(); i++)
			{
				const StaticMeshSlotData& RenderProxy = m_Mesh->Data.MeshesData[i];
				MaterialSlot& Mat = m_Materials[RenderProxy.MaterialIndex];

				if (!Mat.GetParentMaterial()->IsSupportingPrePass())
				{
					continue;
				}

				SCOPE_STAT_DYNAMIC(Mat.GetMaterialName().c_str());

				Mat.GetParentMaterial()->BindPrePass(CommandList);
				Mat.GetMaterialInterface()->BindResources(CommandList);

				m_PrimitiveBuffer.m_LocalToWorld = Matrix(m_OwningInstancedStaticMeshComponent->GetWorldTransform()).Get();
				m_PrimitiveBuffer.m_LocalToProjection = XMMatrixMultiply( m_PrimitiveBuffer.m_LocalToWorld.Get(), Renderer->GetSceneView().WorldToProjection.Get() );
				m_PrimitiveBuffer.m_Guid = m_Guid;
		
				TRefCountPtr<RenderUniformBuffer> MeshBuffer = RenderUniformBuffer::Create(CommandList->GetParentDevice(), sizeof(PrimitiveBuffer), EUniformBufferUsage::SingleFrame, &m_PrimitiveBuffer);
		
				CommandList->SetGraphicRootConstant(Renderer->ViewBuffer->GetViewIndex(), 0);
				CommandList->SetGraphicRootConstant(MeshBuffer->GetViewIndex(), 1);
				CommandList->SetGraphicRootConstant(Renderer::Get()->StaticSamplersBuffer->GetViewIndex(), 2);
		
				RenderProxy.BindAndDraw(CommandList);
			}
		}
	}

	void InstancedStaticMeshSceneProxy::RenderShadowPass( class D3D12CommandList* CommandList, SceneRenderer* Renderer, LightSceneProxy* LightProxy )
	{
		if (m_Mesh.IsValid())
		{
			const std::string MeshName = Path::GetCleanName(m_Mesh.GetPath());
			SCOPE_STAT_DYNAMIC(MeshName.c_str());

			for (size_t i = 0; i < m_Mesh->Data.MeshesData.size(); i++)
			{
				const StaticMeshSlotData& RenderProxy = m_Mesh->Data.MeshesData[i];
				MaterialSlot& Mat = m_Materials[RenderProxy.MaterialIndex];

				if (!Mat.GetParentMaterial()->IsSupportingShadowPass())
				{
					continue;
				}

				SCOPE_STAT_DYNAMIC(Mat.GetMaterialName().c_str());

				if ( LightProxy->GetLightType() == ELightType::PointLight )
				{
					Mat.GetParentMaterial()->BindPointLightShadowDepthPass(CommandList);
				}
				else if ( LightProxy->GetLightType() == ELightType::SpotLight)
				{
					Mat.GetParentMaterial()->BindSpotLightShadowDepthPass(CommandList);
				}
				else if ( LightProxy->GetLightType() == ELightType::DirectionalLight)
				{
					Mat.GetParentMaterial()->BindSpotLightShadowDepthPass(CommandList);
				}

				Mat.GetMaterialInterface()->BindResources(CommandList);

				m_PrimitiveBuffer.m_LocalToWorld = Matrix(m_OwningInstancedStaticMeshComponent->GetWorldTransform()).Get();
				m_PrimitiveBuffer.m_LocalToProjection = XMMatrixMultiply( m_PrimitiveBuffer.m_LocalToWorld.Get(), Renderer->GetSceneView().WorldToProjection.Get() );
				m_PrimitiveBuffer.m_Guid = m_Guid;

				TRefCountPtr<RenderUniformBuffer> MeshBuffer = RenderUniformBuffer::Create(CommandList->GetParentDevice(), sizeof(PrimitiveBuffer), EUniformBufferUsage::SingleFrame, &m_PrimitiveBuffer);

				CommandList->SetGraphicRootConstant(Renderer->ViewBuffer->GetViewIndex(), 0);
				CommandList->SetGraphicRootConstant(MeshBuffer->GetViewIndex(), 1);
				CommandList->SetGraphicRootConstant(Renderer::Get()->StaticSamplersBuffer->GetViewIndex(), 2);

				RenderProxy.BindAndDraw(CommandList);
			}
		}
	}

	void InstancedStaticMeshSceneProxy::RenderDecalPass( class D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		//if (m_Mesh.IsValid())
		//{
		//	const std::string MeshName = Path::GetCleanName(m_Mesh.GetPath());
		//	SCOPE_STAT_DYNAMIC(MeshName.c_str());
		//
		//	for (size_t i = 0; i < m_Mesh->Data.MeshesData.size(); i++)
		//	{
		//		const StaticMeshSlotData& RenderProxy = m_Mesh->Data.MeshesData[i];
		//		MaterialSlot& Mat = m_Materials[RenderProxy.MaterialIndex];
		//		
		//		if (!Mat.GetParentMaterial()->IsSupportingStaticMeshDecalPass())
		//		{
		//			continue;
		//		}
		//
		//		SCOPE_STAT_DYNAMIC(Mat.GetMaterialName().c_str());
		//
		//		Mat.GetParentMaterial()->BindStaticMeshDecalPass(CommandList);
		//		Mat.GetMaterialInterface()->BindResources(CommandList);
		//
		//		m_PrimitiveBuffer.m_LocalToWorld = Matrix(m_OwningInstancedStaticMeshComponent->GetWorldTransform()).Get();
		//		m_PrimitiveBuffer.m_LocalToProjection = XMMatrixMultiply( m_PrimitiveBuffer.m_LocalToWorld.Get(), Renderer->GetSceneView().WorldToProjection.Get() );
		//
		//		TRefCountPtr<RenderUniformBuffer> MeshBuffer = RenderUniformBuffer::Create(CommandList->GetParentDevice(), sizeof(PrimitiveBuffer), EUniformBufferUsage::SingleFrame, &m_PrimitiveBuffer);
		//
		//		CommandList->SetGraphicRootConstant(Renderer->ViewBuffer->GetViewIndex(), 0);
		//		CommandList->SetGraphicRootConstant(MeshBuffer->GetViewIndex(), 1);
		//		CommandList->SetGraphicRootConstant(Renderer::Get()->StaticSamplersBuffer->GetViewIndex(), 2);
		//
		//		RenderProxy.BindAndDraw(CommandList);
		//	}
		//}
	}

#if WITH_EDITOR
	void InstancedStaticMeshSceneProxy::RenderHitProxyPass( class D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		SCOPE_STAT("HitProxyMesh");

		if (!m_Mesh.IsValid() || !m_Selectable)
		{
			return;
		}

		for (size_t i = 0; i < m_Mesh->Data.MeshesData.size(); i++)
		{
			const StaticMeshSlotData& RenderProxy = m_Mesh->Data.MeshesData[i];
			MaterialSlot& Mat = m_Materials[RenderProxy.MaterialIndex];


			if (!Mat.GetParentMaterial()->IsSupportingHitProxyPass())
			{
				continue;
			}

			SCOPE_STAT_DYNAMIC(Mat.GetMaterialName().c_str());

			Mat.GetParentMaterial()->BindHitProxyPass(CommandList);
			Mat.GetMaterialInterface()->BindResources(CommandList);

			{
				SCOPE_STAT("Calculate");

				m_PrimitiveBuffer.m_LocalToWorld = Matrix(m_OwningInstancedStaticMeshComponent->GetWorldTransform()).Get();
				m_PrimitiveBuffer.m_LocalToProjection = XMMatrixMultiply( m_PrimitiveBuffer.m_LocalToWorld.Get(), Renderer->GetSceneView().WorldToProjection.Get() );
				m_PrimitiveBuffer.m_Guid = m_Guid;
			}

			TRefCountPtr<RenderUniformBuffer> MeshBuffer = RenderUniformBuffer::Create(CommandList->GetParentDevice(), sizeof(PrimitiveBuffer), EUniformBufferUsage::SingleFrame, &m_PrimitiveBuffer);

			CommandList->SetGraphicRootConstant(Renderer->ViewBuffer->GetViewIndex(), 0);
			CommandList->SetGraphicRootConstant(MeshBuffer->GetViewIndex(), 1);
			CommandList->SetGraphicRootConstant(Renderer::Get()->StaticSamplersBuffer->GetViewIndex(), 2);

			RenderProxy.BindAndDraw(CommandList);
		}
	}

	void InstancedStaticMeshSceneProxy::RenderSelectionPass( class D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		if (!m_Mesh.IsValid() || !m_SelectedInEditor)
			return;

		const std::string MeshName = Path::GetCleanName(m_Mesh.GetPath());
		SCOPE_STAT_DYNAMIC(MeshName.c_str());

		for (size_t i = 0; i < m_Mesh->Data.MeshesData.size(); i++)
		{
			const StaticMeshSlotData& RenderProxy = m_Mesh->Data.MeshesData[i];
			MaterialSlot& Mat = m_Materials[RenderProxy.MaterialIndex];
		
			if (!Mat.GetParentMaterial()->IsSupportingEditorSelectionPass())
			{
				continue;
			}

			SCOPE_STAT_DYNAMIC(Mat.GetMaterialName().c_str());

			Mat.GetParentMaterial()->BindSelectionPass(CommandList);
			Mat.GetMaterialInterface()->BindResources(CommandList);
		
			m_PrimitiveBuffer.m_LocalToWorld = Matrix(m_OwningInstancedStaticMeshComponent->GetWorldTransform()).Get();
			m_PrimitiveBuffer.m_LocalToProjection = XMMatrixMultiply( m_PrimitiveBuffer.m_LocalToWorld.Get(), Renderer->GetSceneView().WorldToProjection.Get() );
			m_PrimitiveBuffer.m_Guid = m_Guid;

			TRefCountPtr<RenderUniformBuffer> MeshBuffer = RenderUniformBuffer::Create(CommandList->GetParentDevice(), sizeof(PrimitiveBuffer), EUniformBufferUsage::SingleFrame, &m_PrimitiveBuffer);

			CommandList->SetGraphicRootConstant(Renderer->ViewBuffer->GetViewIndex(), 0);
			CommandList->SetGraphicRootConstant(MeshBuffer->GetViewIndex(), 1);
			CommandList->SetGraphicRootConstant(Renderer::Get()->StaticSamplersBuffer->GetViewIndex(), 2);
		
			RenderProxy.BindAndDraw(CommandList);
		}
	}

	void InstancedStaticMeshSceneProxy::RenderEditorPrimitivePass( class D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		if (!m_Mesh.IsValid() || !m_EditorPrimitive)
		{
			return;
		}

		const std::string MeshName = Path::GetCleanName(m_Mesh.GetPath());
		SCOPE_STAT_DYNAMIC(MeshName.c_str());

		for (size_t i = 0; i < m_Mesh->Data.MeshesData.size(); i++)
		{
			const StaticMeshSlotData& RenderProxy = m_Mesh->Data.MeshesData[i];
			MaterialSlot& Mat = m_Materials[RenderProxy.MaterialIndex];

			if (!Mat.GetParentMaterial()->IsSupportingEditorPrimitivePass())
			{
				continue;
			}

			SCOPE_STAT_DYNAMIC(Mat.GetMaterialName().c_str());

			Mat.GetParentMaterial()->BindEditorPrimitivePass(CommandList);
			Mat.GetMaterialInterface()->BindResources(CommandList);
		
			m_PrimitiveBuffer.m_LocalToWorld = Matrix(m_OwningInstancedStaticMeshComponent->GetWorldTransform()).Get();
			m_PrimitiveBuffer.m_LocalToProjection = XMMatrixMultiply( m_PrimitiveBuffer.m_LocalToWorld.Get(), Renderer->GetSceneView().WorldToProjection.Get() );
			m_PrimitiveBuffer.m_Guid = m_Guid;

			TRefCountPtr<RenderUniformBuffer> MeshBuffer = RenderUniformBuffer::Create(CommandList->GetParentDevice(), sizeof(PrimitiveBuffer), EUniformBufferUsage::SingleFrame, &m_PrimitiveBuffer);

			CommandList->SetGraphicRootConstant(Renderer->ViewBuffer->GetViewIndex(), 0);
			CommandList->SetGraphicRootConstant(MeshBuffer->GetViewIndex(), 1);
			CommandList->SetGraphicRootConstant(Renderer::Get()->StaticSamplersBuffer->GetViewIndex(), 2);
		
			RenderProxy.BindAndDraw( CommandList );
		}
	}
#endif

	void InstancedStaticMeshSceneProxy::InitResources( class D3D12CommandList* CommandList )
	{
		
	}

	void InstancedStaticMeshSceneProxy::UpdateResources( class D3D12CommandList* CommandList )
	{
		m_PrimitiveBuffer.m_PrevLocalToWorld = m_PrimitiveBuffer.m_LocalToWorld;
		m_PrimitiveBuffer.m_PrevLocalToProjection = m_PrimitiveBuffer.m_LocalToProjection;

		if (m_OwningInstancedStaticMeshComponent->IsRenderStateDirty())
		{
			m_Mesh = m_OwningInstancedStaticMeshComponent->GetMesh();
		}

		if (m_Mesh.IsValid())
		{
			m_Mesh->UploadResources(CommandList);
		}

		if (m_OwningInstancedStaticMeshComponent->IsRenderStateDirty())
		{
			m_Materials.clear();

			if (m_Mesh.IsValid())
			{
				const uint32 MaterialCount = m_Mesh->Data.Materials.size();
				const uint32 OverrideMaterialCount = m_OwningInstancedStaticMeshComponent->m_OverrideMaterials.size();
				m_Materials.resize(MaterialCount);

				for (int i = 0; i < MaterialCount; i++)
				{
					if (i < OverrideMaterialCount && m_OwningInstancedStaticMeshComponent->m_OverrideMaterials[i].m_Overriden)
					{
						m_Materials[i] = m_OwningInstancedStaticMeshComponent->m_OverrideMaterials[i];
					}
					else
					{
						m_Materials[i] = m_Mesh->Data.Materials[i];
					}

					// TODO: mark this only in with editor builds
					m_Materials[i].LoadChecked();
					if (!m_Materials[i].IsValid())
					{
						//LOG(LogStaticMeshSceneProxy, Error, "Material is invalid. Using default material.");

						m_Materials[i] = AssetHandle<Material>(DEFAULT_MATERIAL_PATH);
						m_Materials[i].Load();
					}
				}
			}
		
		}

		for (MaterialSlot& MatSlot : m_Materials)
		{
			MatSlot.GetMaterialInterface()->UploadResources(CommandList);
		}

		m_OwningInstancedStaticMeshComponent->ClearRenderStateDirty();
	}

}  // namespace Drn