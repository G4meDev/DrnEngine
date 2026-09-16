#include "DrnPCH.h"
#include "DynamicMeshSceneProxy.h"
#include "Runtime/Engine/DynamicMeshComponent.h"

namespace Drn
{
	DynamicMeshSceneProxy::DynamicMeshSceneProxy( DynamicMeshComponent* InDynamicMeshComponent )
		: PrimitiveSceneProxy( InDynamicMeshComponent )
		, m_OwningDynamicMeshComponent( InDynamicMeshComponent )
		, m_HitProxyData(InDynamicMeshComponent)
		, bWasDirty(true)
	{
#if WITH_EDITOR
		m_EditorPrimitive = InDynamicMeshComponent->IsEditorPrimitive();
		m_Selectable = InDynamicMeshComponent->m_Selectable;
#endif

		MinDrawDistance = InDynamicMeshComponent->MinDrawDistance;
		MaxDrawDistance = InDynamicMeshComponent->MaxDrawDistance;
	}

	DynamicMeshSceneProxy::~DynamicMeshSceneProxy()
	{}

	const BoxSphereBounds& DynamicMeshSceneProxy::GetBounds()
	{
		// TODO: only update bounds when dirty

		if (m_OwningDynamicMeshComponent)
		{
			Bounds = m_OwningDynamicMeshComponent->GetBounds();
		}

		return Bounds;
	}

	void DynamicMeshSceneProxy::InitResources( D3D12CommandList* CommandList )
	{}

	void DynamicMeshSceneProxy::UpdateResources( D3D12CommandList* CommandList )
	{
		UpdateDynamicMeshBuffer(CommandList);

		if (m_OwningDynamicMeshComponent->IsRenderStateDirty())
		{
			bWasDirty = true;
			UpdatePrimitiveBuffer(CommandList);
		}

		// @HACK: just one frame delay to propagate PrevLocalToWorld
		if (!m_OwningDynamicMeshComponent->IsRenderStateDirty() && bWasDirty)
		{
			bWasDirty = false;
			UpdatePrimitiveBuffer(CommandList);
		}

		for (MeshSectionRenderData& Section : SectionsRenderData)
		{
			Section.m_Material.GetMaterialInterface()->UploadResources(CommandList);
		}

		m_OwningDynamicMeshComponent->ClearRenderStateDirty();
	}

	void DynamicMeshSceneProxy::UpdateDynamicMeshBuffer( class D3D12CommandList* CommandList )
	{
		std::vector<DynamicMeshSection>& ComponentSections = m_OwningDynamicMeshComponent->MeshSecions;
		const int32 SectionCount = ComponentSections.size();
		SectionsRenderData.resize(SectionCount);

		for (int32 SectionIndex = 0; SectionIndex < SectionCount; SectionIndex++)
		{
			MeshSectionRenderData& RenderData = SectionsRenderData[SectionIndex];
			DynamicMeshSection& ComponentSection = ComponentSections[SectionIndex];

			RenderData.m_Material = ComponentSection.SectionMaterial;
			RenderData.bVisible = ComponentSection.bSectionVisible;

			if (ComponentSection.bSectionDirty)
			{
				StaticMeshVertexData& VData = ComponentSection.VertexBufferData;
				if (VData.GetVertexCount() > 0 && VData.GetIndexCount() > 0)
				{
					RenderData.m_VertexBuffer = StaticMeshVertexBuffer::Create(CommandList, VData, "VB_DynamicMesh", true);

					uint32 IndexBufferFlags = (uint32)EBufferUsageFlags::IndexBuffer | (uint32)EBufferUsageFlags::Static;
					RenderResourceCreateInfo IndexBufferCreateInfo(nullptr, VData.GetIndexBufferPtr(), ClearValueBinding::Black, "IB_DynamicMesh");
					RenderData.m_IndexBuffer = RenderIndexBuffer::Create(Renderer::Get()->GetDevice(), CommandList, VData.GetIndexBufferStride(), VData.GetIndexBufferByteSize(),
						IndexBufferFlags, D3D12_RESOURCE_STATE_COMMON, false, IndexBufferCreateInfo);

					RenderData.VertexCount = ComponentSection.VertexBufferData.GetVertexCount();
					RenderData.PrimitiveCount = ComponentSection.VertexBufferData.GetPrimitiveCount();
				}
				else
				{
					RenderData.m_VertexBuffer = nullptr;
					RenderData.m_IndexBuffer = nullptr;
				}

				ComponentSection.bSectionDirty = false;
			}
		}
	}

	void DynamicMeshSceneProxy::UpdatePrimitiveBuffer( D3D12CommandList* CommandList )
	{
		// TODO: issue when proxy not begin rendered
		m_PrimitiveData.m_PrevLocalToWorld = m_PrimitiveData.m_LocalToWorld;
		m_PrimitiveData.m_LocalToWorld = Matrix(m_OwningDynamicMeshComponent->GetWorldTransform()).Get();
		m_PrimitiveData.m_HitProxyData = m_HitProxyData;

		PrimitiveBuffer = RenderUniformBuffer::Create(CommandList->GetParentDevice(), sizeof(PrimitiveData), EUniformBufferUsage::MultiFrame, &m_PrimitiveData);
	}

	PrimitiveComponent* DynamicMeshSceneProxy::GetPrimitive() { return m_OwningDynamicMeshComponent; }

	void DynamicMeshSceneProxy::RenderVelocityPass( class D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		SCOPE_STAT();
		
		for (const MeshSectionRenderData& RenderData : SectionsRenderData)
		{
			if (RenderData.IsValid() && RenderData.bVisible)
			{
				const MaterialSlot& Mat = RenderData.m_Material;
				MaterialShader* MatShader = Mat.GetParentMaterial()->GetShaderParameters().bIsUsedWithStaticMesh && Mat.GetParentMaterial()->GetShaderParameters().bHasVelocityPass
					? Mat.GetParentMaterial()->GetShaders().GetShader(VertexFactoryType::StaticMesh, EMaterialStage::Velocity)
					: nullptr;
		
				if (MatShader)
				{
					SCOPE_STAT_DYNAMIC(Mat.GetMaterialName().c_str());
		
					MatShader->Bind(CommandList);
					Mat.GetMaterialInterface()->BindResources(CommandList);
		
					CommandList->SetGraphicRootConstant(Renderer->ViewBuffer->GetViewIndex(), 0);
					CommandList->SetGraphicRootConstant(PrimitiveBuffer->GetViewIndex(), 1);
					CommandList->SetGraphicRootConstant(Renderer::Get()->StaticSamplersBuffer->GetViewIndex(), 2);
		
					RenderData.BindAndDraw(CommandList);
				}
			}
		}
	}

	void DynamicMeshSceneProxy::RenderTranslucencyPass( class D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		SCOPE_STAT();

		for (const MeshSectionRenderData& RenderData : SectionsRenderData)
		{
			if (RenderData.IsValid() && RenderData.bVisible)
			{
				const MaterialSlot& Mat = RenderData.m_Material;
				MaterialShader* MatShader = Mat.GetParentMaterial()->GetShaderParameters().bIsUsedWithStaticMesh && Mat.GetParentMaterial()->GetShaderParameters().bHasTranslucencyPass
					? Mat.GetParentMaterial()->GetShaders().GetShader(VertexFactoryType::StaticMesh, EMaterialStage::Translucensy)
					: nullptr;
		
				if (MatShader)
				{
					SCOPE_STAT_DYNAMIC(Mat.GetMaterialName().c_str());
		
					MatShader->Bind(CommandList);
					Mat.GetMaterialInterface()->BindResources(CommandList);
		
					CommandList->SetGraphicRootConstant(Renderer->ViewBuffer->GetViewIndex(), 0);
					CommandList->SetGraphicRootConstant(PrimitiveBuffer->GetViewIndex(), 1);
					CommandList->SetGraphicRootConstant(Renderer::Get()->StaticSamplersBuffer->GetViewIndex(), 2);
		
					RenderData.BindAndDraw(CommandList);
				}
			}
		}
	}

	void DynamicMeshSceneProxy::RenderDistortionPass( class D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		SCOPE_STAT();

		for (const MeshSectionRenderData& RenderData : SectionsRenderData)
		{
			if (RenderData.IsValid() && RenderData.bVisible)
			{
				const MaterialSlot& Mat = RenderData.m_Material;
				MaterialShader* MatShader = Mat.GetParentMaterial()->GetShaderParameters().bIsUsedWithStaticMesh && Mat.GetParentMaterial()->GetShaderParameters().bHasDistortionPass
					? Mat.GetParentMaterial()->GetShaders().GetShader(VertexFactoryType::StaticMesh, EMaterialStage::Distortion)
					: nullptr;
		
				if (MatShader)
				{
					SCOPE_STAT_DYNAMIC(Mat.GetMaterialName().c_str());
		
					MatShader->Bind(CommandList);
					Mat.GetMaterialInterface()->BindResources(CommandList);
		
					CommandList->SetGraphicRootConstant(Renderer->ViewBuffer->GetViewIndex(), 0);
					CommandList->SetGraphicRootConstant(PrimitiveBuffer->GetViewIndex(), 1);
					CommandList->SetGraphicRootConstant(Renderer::Get()->StaticSamplersBuffer->GetViewIndex(), 2);
		
					RenderData.BindAndDraw(CommandList);
				}
			}
		}
	}

	void DynamicMeshSceneProxy::RenderMainPass( D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		SCOPE_STAT();

		for (const MeshSectionRenderData& RenderData : SectionsRenderData)
		{
			if (RenderData.IsValid() && RenderData.bVisible)
			{
				const MaterialSlot& Mat = RenderData.m_Material;
				MaterialShader* MatShader = Mat.GetParentMaterial()->GetShaderParameters().bIsUsedWithStaticMesh && Mat.GetParentMaterial()->GetShaderParameters().bHasMainPass
					? Mat.GetParentMaterial()->GetShaders().GetShader(VertexFactoryType::StaticMesh, EMaterialStage::Main)
					: nullptr;

				if (MatShader)
				{
					SCOPE_STAT_DYNAMIC(Mat.GetMaterialName().c_str());

					MatShader->Bind(CommandList);
					Mat.GetMaterialInterface()->BindResources(CommandList);

					CommandList->SetGraphicRootConstant(Renderer->ViewBuffer->GetViewIndex(), 0);
					CommandList->SetGraphicRootConstant(PrimitiveBuffer->GetViewIndex(), 1);
					CommandList->SetGraphicRootConstant(Renderer::Get()->StaticSamplersBuffer->GetViewIndex(), 2);

					RenderData.BindAndDraw(CommandList);
				}
			}
		}
	}

	void DynamicMeshSceneProxy::RenderPrePass( D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		SCOPE_STAT();
		for (const MeshSectionRenderData& RenderData : SectionsRenderData)
		{
			if (RenderData.IsValid() && RenderData.bVisible)
			{
				const MaterialSlot& Mat = RenderData.m_Material;
				MaterialShader* MatShader = nullptr;
				if (Mat.GetParentMaterial()->GetShaderParameters().bIsUsedWithStaticMesh && Mat.GetParentMaterial()->GetShaderParameters().bHasPrepass)
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
		
					CommandList->SetGraphicRootConstant(Renderer->ViewBuffer->GetViewIndex(), 0);
					CommandList->SetGraphicRootConstant(PrimitiveBuffer->GetViewIndex(), 1);
					CommandList->SetGraphicRootConstant(Renderer::Get()->StaticSamplersBuffer->GetViewIndex(), 2);
		
					RenderData.BindAndDraw(CommandList);
				}
			}
		}
	}

	void DynamicMeshSceneProxy::RenderShadowPass( D3D12CommandList* CommandList, SceneRenderer* Renderer, LightSceneProxy* LightProxy)
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
		
		SCOPE_STAT();
		for (const MeshSectionRenderData& RenderData : SectionsRenderData)
		{
			if (RenderData.IsValid() && RenderData.bVisible)
			{
				const MaterialSlot& Mat = RenderData.m_Material;
				MaterialShader* MatShader = Mat.GetParentMaterial()->GetShaderParameters().bIsUsedWithStaticMesh && Mat.GetParentMaterial()->GetShaderParameters().bHasShadowPass
					? GetMaterialShaderForLightType(Mat.GetParentMaterial()->GetShaders(), LightProxy->GetLightType())
					: nullptr;
		
				if (MatShader)
				{
					SCOPE_STAT_DYNAMIC(Mat.GetMaterialName().c_str());
		
					MatShader->Bind(CommandList);
					Mat.GetMaterialInterface()->BindResources(CommandList);
		
					CommandList->SetGraphicRootConstant(Renderer->ViewBuffer->GetViewIndex(), 0);
					CommandList->SetGraphicRootConstant(PrimitiveBuffer->GetViewIndex(), 1);
					CommandList->SetGraphicRootConstant(Renderer::Get()->StaticSamplersBuffer->GetViewIndex(), 2);
		
					RenderData.BindAndDraw(CommandList);
				}
			}
		}
	}

	void DynamicMeshSceneProxy::RenderDecalPass( D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		SCOPE_STAT();

		for (const MeshSectionRenderData& RenderData : SectionsRenderData)
		{
			if (RenderData.IsValid() && RenderData.bVisible)
			{
				const MaterialSlot& Mat = RenderData.m_Material;
				MaterialShader* MatShader = Mat.GetParentMaterial()->GetShaderParameters().bIsUsedWithStaticMesh && Mat.GetParentMaterial()->GetShaderParameters().bHasDecalPass
					? Mat.GetParentMaterial()->GetShaders().GetShader(VertexFactoryType::StaticMesh, EMaterialStage::Decal)
					: nullptr;
		
				if (MatShader)
				{
					SCOPE_STAT_DYNAMIC(Mat.GetMaterialName().c_str());
		
					MatShader->Bind(CommandList);
					Mat.GetMaterialInterface()->BindResources(CommandList);
		
					CommandList->SetGraphicRootConstant(Renderer->ViewBuffer->GetViewIndex(), 0);
					CommandList->SetGraphicRootConstant(PrimitiveBuffer->GetViewIndex(), 1);
					CommandList->SetGraphicRootConstant(Renderer::Get()->StaticSamplersBuffer->GetViewIndex(), 2);
		
					RenderData.BindAndDraw(CommandList);
				}
			}
		}
	}

#if WITH_EDITOR

	void DynamicMeshSceneProxy::RenderHitProxyPass( D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		SCOPE_STAT("HitProxyMesh");
		
		for (const MeshSectionRenderData& RenderData : SectionsRenderData)
		{
			if (RenderData.IsValid() && RenderData.bVisible && m_Selectable)
			{
				const MaterialSlot& Mat = RenderData.m_Material;
				MaterialShader* MatShader = Mat.GetParentMaterial()->GetShaderParameters().bIsUsedWithStaticMesh && Mat.GetParentMaterial()->GetShaderParameters().bHasHitProxyPass
					? Mat.GetParentMaterial()->GetShaders().GetShader(VertexFactoryType::StaticMesh, EMaterialStage::Hitproxy)
					: nullptr;
		
				if (MatShader)
				{
					SCOPE_STAT_DYNAMIC(Mat.GetMaterialName().c_str());
		
					MatShader->Bind(CommandList);
					Mat.GetMaterialInterface()->BindResources(CommandList);
		
					CommandList->SetGraphicRootConstant(Renderer->ViewBuffer->GetViewIndex(), 0);
					CommandList->SetGraphicRootConstant(PrimitiveBuffer->GetViewIndex(), 1);
					CommandList->SetGraphicRootConstant(Renderer::Get()->StaticSamplersBuffer->GetViewIndex(), 2);
		
					RenderData.BindAndDraw(CommandList);
				}
			}
		}
	}

	void DynamicMeshSceneProxy::RenderSelectionPass( D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		SCOPE_STAT();
		
		for (const MeshSectionRenderData& RenderData : SectionsRenderData)
		{
			if (RenderData.IsValid() && RenderData.bVisible && m_SelectedInEditor)
			{
				const MaterialSlot& Mat = RenderData.m_Material;
				MaterialShader* MatShader = Mat.GetParentMaterial()->GetShaderParameters().bIsUsedWithStaticMesh && Mat.GetParentMaterial()->GetShaderParameters().bHasEditorSelectionPass
					? Mat.GetParentMaterial()->GetShaders().GetShader(VertexFactoryType::StaticMesh, EMaterialStage::EditorSelection)
					: nullptr;
				if (MatShader)
				{
					SCOPE_STAT_DYNAMIC(Mat.GetMaterialName().c_str());
		
					MatShader->Bind(CommandList);
					Mat.GetMaterialInterface()->BindResources(CommandList);
		
					CommandList->SetGraphicRootConstant(Renderer->ViewBuffer->GetViewIndex(), 0);
					CommandList->SetGraphicRootConstant(PrimitiveBuffer->GetViewIndex(), 1);
					CommandList->SetGraphicRootConstant(Renderer::Get()->StaticSamplersBuffer->GetViewIndex(), 2);
		
					RenderData.BindAndDraw(CommandList);
				}
			}
		}
	}

	void DynamicMeshSceneProxy::RenderEditorPrimitivePass( D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		SCOPE_STAT();

		for (const MeshSectionRenderData& RenderData : SectionsRenderData)
		{
			if (RenderData.IsValid() && RenderData.bVisible && m_EditorPrimitive)
			{
				const MaterialSlot& Mat = RenderData.m_Material;
				MaterialShader* MatShader = Mat.GetParentMaterial()->GetShaderParameters().bIsUsedWithStaticMesh && Mat.GetParentMaterial()->GetShaderParameters().bHasEditorPrimitivePass
					? Mat.GetParentMaterial()->GetShaders().GetShader(VertexFactoryType::StaticMesh, EMaterialStage::EditorPrimitive)
					: nullptr;
				if (MatShader)
				{
					SCOPE_STAT_DYNAMIC(Mat.GetMaterialName().c_str());
		
					MatShader->Bind(CommandList);
					Mat.GetMaterialInterface()->BindResources(CommandList);
		
					CommandList->SetGraphicRootConstant(Renderer->ViewBuffer->GetViewIndex(), 0);
					CommandList->SetGraphicRootConstant(PrimitiveBuffer->GetViewIndex(), 1);
					CommandList->SetGraphicRootConstant(Renderer::Get()->StaticSamplersBuffer->GetViewIndex(), 2);
		
					RenderData.BindAndDraw(CommandList);
				}
			}
		}
	}

#endif

}