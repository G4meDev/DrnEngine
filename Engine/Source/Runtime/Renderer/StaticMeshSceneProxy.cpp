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

		MinDrawDistance = InStaticMeshComponent->MinDrawDistance;
		MaxDrawDistance = InStaticMeshComponent->MaxDrawDistance;
	}

	StaticMeshSceneProxy::~StaticMeshSceneProxy()
	{}

	const BoxSphereBounds& StaticMeshSceneProxy::GetBounds()
	{
		// TODO: only update bounds when dirty

		if (m_OwningStaticMeshComponent)
		{
			Bounds = m_OwningStaticMeshComponent->GetBounds();
		}

		return Bounds;
	}

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
						m_Materials[i] = m_OwningStaticMeshComponent->m_OverrideMaterials[i];
					}
					else
					{
						m_Materials[i] = m_Mesh->Data.Materials[i];
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

		for (MaterialSlot& MatSlot : m_Materials)
		{
			MatSlot.GetMaterialInterface()->UploadResources(CommandList);
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
				MaterialSlot& Mat = m_Materials[RenderProxy.MaterialIndex];
				
				MaterialShader* MatShader = Mat.GetParentMaterial()->GetShaderParameters().bHasMainPass
					? Mat.GetParentMaterial()->GetShaders().GetShader(VertexFactoryType::StaticMesh, EMaterialStage::Main)
					: nullptr;

				if (MatShader)
				{
					SCOPE_STAT_DYNAMIC(Mat.GetMaterialName().c_str());

					MatShader->Bind(CommandList);
					CommandList->GetD3D12CommandList()->OMSetStencilRef(255);
					Mat.GetMaterialInterface()->BindResources(CommandList);

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
				MaterialSlot& Mat = m_Materials[RenderProxy.MaterialIndex];

				MaterialShader* MatShader = nullptr;
				if (Mat.GetParentMaterial()->GetShaderParameters().bHasPrepass)
				{
					MatShader = Mat.GetParentMaterial()->GetShaderParameters().bHasCustomPrepass
						? Mat.GetParentMaterial()->GetShaders().GetShader(VertexFactoryType::StaticMesh, EMaterialStage::Prepass)
						: CommonResources::Get()->m_PositionOnlyMaterialShaders.GetShader(VertexFactoryType::StaticMesh, Mat.GetParentMaterial()->IsTwoSided());
				}

				if (MatShader)
				{
					SCOPE_STAT_DYNAMIC(Mat.GetMaterialName().c_str());

					MatShader->Bind(CommandList);
					Mat.GetMaterialInterface()->BindResources(CommandList);

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
	}

	void StaticMeshSceneProxy::RenderShadowPass( D3D12CommandList* CommandList, SceneRenderer* Renderer, LightSceneProxy* LightProxy)
	{
		auto GetMaterialShaderForLightType = [](const MaterialShaders& Shaders, ELightType Type)
		{
			switch ( Type )
			{
			case ELightType::PointLight: return Shaders.GetShader(VertexFactoryType::StaticMesh, EMaterialStage::PointLightShadow);
			case ELightType::SpotLight:
			case ELightType::DirectionalLight: return Shaders.GetShader(VertexFactoryType::StaticMesh, EMaterialStage::SpotLightShadow);
			case ELightType::SkyLight:
			default: drn_check(false); return Shaders.GetShader(VertexFactoryType::StaticMesh, EMaterialStage::PointLightShadow);
			}
		};

		if (m_Mesh.IsValid())
		{
			const std::string MeshName = Path::GetCleanName(m_Mesh.GetPath());
			SCOPE_STAT_DYNAMIC(MeshName.c_str());

			for (size_t i = 0; i < m_Mesh->Data.MeshesData.size(); i++)
			{
				const StaticMeshSlotData& RenderProxy = m_Mesh->Data.MeshesData[i];
				MaterialSlot& Mat = m_Materials[RenderProxy.MaterialIndex];

				MaterialShader* MatShader = Mat.GetParentMaterial()->GetShaderParameters().bHasShadowPass
					? GetMaterialShaderForLightType(Mat.GetParentMaterial()->GetShaders(), LightProxy->GetLightType())
					: nullptr;

				if (MatShader)
				{
					SCOPE_STAT_DYNAMIC(Mat.GetMaterialName().c_str());

					MatShader->Bind(CommandList);
					Mat.GetMaterialInterface()->BindResources(CommandList);

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
				MaterialSlot& Mat = m_Materials[RenderProxy.MaterialIndex];

				MaterialShader* MatShader = Mat.GetParentMaterial()->GetShaderParameters().bHasStaticMeshDecalPass
					? Mat.GetParentMaterial()->GetShaders().GetShader(VertexFactoryType::StaticMesh, EMaterialStage::StaticMeshDecal)
					: nullptr;

				if (MatShader)
				{
					SCOPE_STAT_DYNAMIC(Mat.GetMaterialName().c_str());

					MatShader->Bind(CommandList);
					Mat.GetMaterialInterface()->BindResources(CommandList);

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
	}

#if WITH_EDITOR

	void StaticMeshSceneProxy::RenderHitProxyPass( D3D12CommandList* CommandList, SceneRenderer* Renderer )
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

			MaterialShader* MatShader = Mat.GetParentMaterial()->GetShaderParameters().bHasHitProxyPass
				? Mat.GetParentMaterial()->GetShaders().GetShader(VertexFactoryType::StaticMesh, EMaterialStage::Hitproxy)
				: nullptr;

			if (MatShader)
			{
				SCOPE_STAT_DYNAMIC(Mat.GetMaterialName().c_str());

				MatShader->Bind(CommandList);
				Mat.GetMaterialInterface()->BindResources(CommandList);

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
	}

	void StaticMeshSceneProxy::RenderSelectionPass( D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		if (!m_Mesh.IsValid() || !m_SelectedInEditor)
			return;

		const std::string MeshName = Path::GetCleanName(m_Mesh.GetPath());
		SCOPE_STAT_DYNAMIC(MeshName.c_str());

		for (size_t i = 0; i < m_Mesh->Data.MeshesData.size(); i++)
		{
			const StaticMeshSlotData& RenderProxy = m_Mesh->Data.MeshesData[i];
			MaterialSlot& Mat = m_Materials[RenderProxy.MaterialIndex];

			MaterialShader* MatShader = Mat.GetParentMaterial()->GetShaderParameters().bHasEditorSelectionPass
				? Mat.GetParentMaterial()->GetShaders().GetShader(VertexFactoryType::StaticMesh, EMaterialStage::EditorSelection)
				: nullptr;
			if (MatShader)
			{
				SCOPE_STAT_DYNAMIC(Mat.GetMaterialName().c_str());

				MatShader->Bind(CommandList);
				CommandList->GetD3D12CommandList()->OMSetStencilRef(255);
				Mat.GetMaterialInterface()->BindResources(CommandList);
		
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

	void StaticMeshSceneProxy::RenderEditorPrimitivePass( D3D12CommandList* CommandList, SceneRenderer* Renderer )
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

			MaterialShader* MatShader = Mat.GetParentMaterial()->GetShaderParameters().bHasEditorPrimitivePass
				? Mat.GetParentMaterial()->GetShaders().GetShader(VertexFactoryType::StaticMesh, EMaterialStage::EditorPrimitive)
				: nullptr;
			if (MatShader)
			{
				SCOPE_STAT_DYNAMIC(Mat.GetMaterialName().c_str());

				MatShader->Bind(CommandList);
				Mat.GetMaterialInterface()->BindResources(CommandList);
		
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