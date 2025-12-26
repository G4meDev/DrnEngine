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
#if WITH_EDITOR
		m_EditorPrimitive = InStaticMeshComponent->IsEditorPrimitive();
		m_Selectable = InStaticMeshComponent->m_Selectable;
#endif
	}

	StaticMeshSceneProxy::~StaticMeshSceneProxy()
	{}

	void StaticMeshSceneProxy::InitResources( D3D12CommandList* CommandList )
	{}

	void StaticMeshSceneProxy::UpdateResources( D3D12CommandList* CommandList )
	{
		m_PrimitiveBuffer.m_PrevLocalToWorld = m_PrimitiveBuffer.m_LocalToWorld;
		m_PrimitiveBuffer.m_PrevLocalToProjection = m_PrimitiveBuffer.m_LocalToProjection;

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

	void StaticMeshSceneProxy::RenderMainPass( D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		if (m_Mesh.IsValid())
		{
			const std::string MeshName = Path::GetCleanName(m_Mesh.GetPath());
			SCOPE_STAT_DYNAMIC(MeshName.c_str());

			for (size_t i = 0; i < m_Mesh->Data.MeshesData.size(); i++)
			{
				const StaticMeshSlotData& RenderProxy = m_Mesh->Data.MeshesData[i];
				AssetHandle<Material>& Mat = m_Materials[RenderProxy.MaterialIndex];
				
				if (!Mat->IsSupportingBasePass())
				{
					continue;
				}

				const std::string MaterialName = Path::GetCleanName(Mat.GetPath());
				SCOPE_STAT_DYNAMIC(MaterialName.c_str());

				Mat->BindMainPass(CommandList);

				m_PrimitiveBuffer.m_LocalToWorld = Matrix(m_OwningStaticMeshComponent->GetWorldTransform()).Get();
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

	void StaticMeshSceneProxy::RenderPrePass( D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		if (m_Mesh.IsValid())
		{
			const std::string MeshName = Path::GetCleanName(m_Mesh.GetPath());
			SCOPE_STAT_DYNAMIC(MeshName.c_str());

			for (size_t i = 0; i < m_Mesh->Data.MeshesData.size(); i++)
			{
				const StaticMeshSlotData& RenderProxy = m_Mesh->Data.MeshesData[i];
				AssetHandle<Material>& Mat = m_Materials[RenderProxy.MaterialIndex];

				if (!Mat->IsSupportingPrePass())
				{
					continue;
				}

				const std::string MaterialName = Path::GetCleanName(Mat.GetPath());
				SCOPE_STAT_DYNAMIC(MaterialName.c_str());

				Mat->BindPrePass(CommandList);

				m_PrimitiveBuffer.m_LocalToWorld = Matrix(m_OwningStaticMeshComponent->GetWorldTransform()).Get();
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

	void StaticMeshSceneProxy::RenderShadowPass( D3D12CommandList* CommandList, SceneRenderer* Renderer, LightSceneProxy* LightProxy, uint32 ShadowBufferIndex )
	{
		if (m_Mesh.IsValid())
		{
			const std::string MeshName = Path::GetCleanName(m_Mesh.GetPath());
			SCOPE_STAT_DYNAMIC(MeshName.c_str());

			for (size_t i = 0; i < m_Mesh->Data.MeshesData.size(); i++)
			{
				const StaticMeshSlotData& RenderProxy = m_Mesh->Data.MeshesData[i];
				AssetHandle<Material>& Mat = m_Materials[RenderProxy.MaterialIndex];

				if (!Mat->IsSupportingShadowPass())
				{
					continue;
				}

				const std::string MaterialName = Path::GetCleanName(Mat.GetPath());
				SCOPE_STAT_DYNAMIC(MaterialName.c_str());

				if ( LightProxy->GetLightType() == ELightType::PointLight )
				{
					Mat->BindPointLightShadowDepthPass(CommandList);
				}
				else if ( LightProxy->GetLightType() == ELightType::SpotLight)
				{
					Mat->BindSpotLightShadowDepthPass(CommandList);
				}
				else if ( LightProxy->GetLightType() == ELightType::DirectionalLight)
				{
					Mat->BindSpotLightShadowDepthPass(CommandList);
				}

				m_PrimitiveBuffer.m_LocalToWorld = Matrix(m_OwningStaticMeshComponent->GetWorldTransform()).Get();
				m_PrimitiveBuffer.m_LocalToProjection = XMMatrixMultiply( m_PrimitiveBuffer.m_LocalToWorld.Get(), Renderer->GetSceneView().WorldToProjection.Get() );
				m_PrimitiveBuffer.m_Guid = m_Guid;

				TRefCountPtr<RenderUniformBuffer> MeshBuffer = RenderUniformBuffer::Create(CommandList->GetParentDevice(), sizeof(PrimitiveBuffer), EUniformBufferUsage::SingleFrame, &m_PrimitiveBuffer);

				CommandList->SetGraphicRootConstant(Renderer->ViewBuffer->GetViewIndex(), 0);
				CommandList->SetGraphicRootConstant(MeshBuffer->GetViewIndex(), 1);
				CommandList->SetGraphicRootConstant(Renderer::Get()->StaticSamplersBuffer->GetViewIndex(), 2);

				CommandList->SetGraphicRootConstant(ShadowBufferIndex, 6);

				RenderProxy.BindAndDraw(CommandList);
			}
		}
	}

	void StaticMeshSceneProxy::RenderDecalPass( D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		if (m_Mesh.IsValid())
		{
			const std::string MeshName = Path::GetCleanName(m_Mesh.GetPath());
			SCOPE_STAT_DYNAMIC(MeshName.c_str());

			for (size_t i = 0; i < m_Mesh->Data.MeshesData.size(); i++)
			{
				const StaticMeshSlotData& RenderProxy = m_Mesh->Data.MeshesData[i];
				AssetHandle<Material>& Mat = m_Materials[RenderProxy.MaterialIndex];
				
				if (!Mat->IsSupportingStaticMeshDecalPass())
				{
					continue;
				}

				const std::string MaterialName = Path::GetCleanName(Mat.GetPath());
				SCOPE_STAT_DYNAMIC(MaterialName.c_str());

				Mat->BindStaticMeshDecalPass(CommandList);

				m_PrimitiveBuffer.m_LocalToWorld = Matrix(m_OwningStaticMeshComponent->GetWorldTransform()).Get();
				m_PrimitiveBuffer.m_LocalToProjection = XMMatrixMultiply( m_PrimitiveBuffer.m_LocalToWorld.Get(), Renderer->GetSceneView().WorldToProjection.Get() );

				TRefCountPtr<RenderUniformBuffer> MeshBuffer = RenderUniformBuffer::Create(CommandList->GetParentDevice(), sizeof(PrimitiveBuffer), EUniformBufferUsage::SingleFrame, &m_PrimitiveBuffer);

				CommandList->SetGraphicRootConstant(Renderer->ViewBuffer->GetViewIndex(), 0);
				CommandList->SetGraphicRootConstant(MeshBuffer->GetViewIndex(), 1);
				CommandList->SetGraphicRootConstant(Renderer::Get()->StaticSamplersBuffer->GetViewIndex(), 2);

				RenderProxy.BindAndDraw(CommandList);
			}
		}
	}

#if WITH_EDITOR

	void StaticMeshSceneProxy::RenderHitProxyPass( D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		SCOPE_STAT("HitProxyMesh");

		if (!m_Selectable)
		{
			return;
		}

		for (size_t i = 0; i < m_Mesh->Data.MeshesData.size(); i++)
		{
			const StaticMeshSlotData& RenderProxy = m_Mesh->Data.MeshesData[i];
			AssetHandle<Material>& Mat = m_Materials[RenderProxy.MaterialIndex];

			const std::string MeshName = Path::GetCleanName(m_Mesh.GetPath());
			SCOPE_STAT_DYNAMIC(MeshName.c_str());

			if (!Mat->IsSupportingHitProxyPass())
			{
				continue;
			}

			const std::string MaterialName = Path::GetCleanName(Mat.GetPath());
			SCOPE_STAT_DYNAMIC(MaterialName.c_str());

			Mat->BindHitProxyPass(CommandList);

			{
				SCOPE_STAT("Calculate");

				m_PrimitiveBuffer.m_LocalToWorld = Matrix(m_OwningStaticMeshComponent->GetWorldTransform()).Get();
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

	void StaticMeshSceneProxy::RenderSelectionPass( D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		if (!m_SelectedInEditor)
			return;

		const std::string MeshName = Path::GetCleanName(m_Mesh.GetPath());
		SCOPE_STAT_DYNAMIC(MeshName.c_str());

		for (size_t i = 0; i < m_Mesh->Data.MeshesData.size(); i++)
		{
			const StaticMeshSlotData& RenderProxy = m_Mesh->Data.MeshesData[i];
			AssetHandle<Material>& Mat = m_Materials[RenderProxy.MaterialIndex];
		
			if (!Mat->IsSupportingEditorSelectionPass())
			{
				continue;
			}

			const std::string MaterialName = Path::GetCleanName(Mat.GetPath());
			SCOPE_STAT_DYNAMIC(MaterialName.c_str());

			Mat->BindSelectionPass(CommandList);
		
			m_PrimitiveBuffer.m_LocalToWorld = Matrix(m_OwningStaticMeshComponent->GetWorldTransform()).Get();
			m_PrimitiveBuffer.m_LocalToProjection = XMMatrixMultiply( m_PrimitiveBuffer.m_LocalToWorld.Get(), Renderer->GetSceneView().WorldToProjection.Get() );
			m_PrimitiveBuffer.m_Guid = m_Guid;

			TRefCountPtr<RenderUniformBuffer> MeshBuffer = RenderUniformBuffer::Create(CommandList->GetParentDevice(), sizeof(PrimitiveBuffer), EUniformBufferUsage::SingleFrame, &m_PrimitiveBuffer);

			CommandList->SetGraphicRootConstant(Renderer->ViewBuffer->GetViewIndex(), 0);
			CommandList->SetGraphicRootConstant(MeshBuffer->GetViewIndex(), 1);
			CommandList->SetGraphicRootConstant(Renderer::Get()->StaticSamplersBuffer->GetViewIndex(), 2);
		
			RenderProxy.BindAndDraw(CommandList);
		}
	}

	void StaticMeshSceneProxy::RenderEditorPrimitivePass( D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		if (!m_EditorPrimitive)
		{
			return;
		}

		if (m_Mesh.IsValid())
		{
			const std::string MeshName = Path::GetCleanName(m_Mesh.GetPath());
			SCOPE_STAT_DYNAMIC(MeshName.c_str());

			for (size_t i = 0; i < m_Mesh->Data.MeshesData.size(); i++)
			{
				const StaticMeshSlotData& RenderProxy = m_Mesh->Data.MeshesData[i];
				AssetHandle<Material>& Mat = m_Materials[RenderProxy.MaterialIndex];
				
				if (!Mat->IsSupportingEditorPrimitivePass())
				{
					continue;
				}

				const std::string MaterialName = Path::GetCleanName(Mat.GetPath());
				SCOPE_STAT_DYNAMIC(MaterialName.c_str());

				Mat->BindEditorPrimitivePass(CommandList);
		
				m_PrimitiveBuffer.m_LocalToWorld = Matrix(m_OwningStaticMeshComponent->GetWorldTransform()).Get();
				m_PrimitiveBuffer.m_LocalToProjection = XMMatrixMultiply( m_PrimitiveBuffer.m_LocalToWorld.Get(), Renderer->GetSceneView().WorldToProjection.Get() );
				m_PrimitiveBuffer.m_Guid = m_Guid;

				TRefCountPtr<RenderUniformBuffer> MeshBuffer = RenderUniformBuffer::Create(CommandList->GetParentDevice(), sizeof(PrimitiveBuffer), EUniformBufferUsage::SingleFrame, &m_PrimitiveBuffer);

				CommandList->SetGraphicRootConstant(Renderer->ViewBuffer->GetViewIndex(), 0);
				CommandList->SetGraphicRootConstant(MeshBuffer->GetViewIndex(), 1);
				CommandList->SetGraphicRootConstant(Renderer::Get()->StaticSamplersBuffer->GetViewIndex(), 2);
		
				RenderProxy.BindAndDraw( CommandList );
			}
		}
	}

#endif

}