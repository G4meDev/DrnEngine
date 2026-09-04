#include "DrnPCH.h"
#include "SkeletalMeshSceneProxy.h"

LOG_DEFINE_CATEGORY( LogSkeletalMeshSceneProxy, "SkeletalMeshSceneProxy" );

namespace Drn
{
	SkeletalMeshSceneProxy::SkeletalMeshSceneProxy( SkeletalMeshComponent* InSkeletalMeshComponent )
		: PrimitiveSceneProxy( InSkeletalMeshComponent )
		, m_OwningSkeletalMeshComponent( InSkeletalMeshComponent )
		, m_HitProxyData(InSkeletalMeshComponent)
		, bWasDirty(true)
	{
#if WITH_EDITOR
		m_EditorPrimitive = InSkeletalMeshComponent->IsEditorPrimitive();
		m_Selectable = InSkeletalMeshComponent->m_Selectable;
#endif

		MinDrawDistance = InSkeletalMeshComponent->MinDrawDistance;
		MaxDrawDistance = InSkeletalMeshComponent->MaxDrawDistance;
	}

	SkeletalMeshSceneProxy::~SkeletalMeshSceneProxy()
	{}

	const BoxSphereBounds& SkeletalMeshSceneProxy::GetBounds()
	{
		// TODO: only update bounds when dirty

		if (m_OwningSkeletalMeshComponent)
		{
			Bounds = m_OwningSkeletalMeshComponent->GetBounds();
		}

		return Bounds;
	}

	void SkeletalMeshSceneProxy::InitResources( D3D12CommandList* CommandList )
	{}

	void SkeletalMeshSceneProxy::UpdateResources( D3D12CommandList* CommandList )
	{
		//if (m_OwningSkeletalMeshComponent->IsRenderStateDirty())
		//{
		//	m_Mesh = m_OwningSkeletalMeshComponent->GetMesh();
		//}
		//
		//if (m_Mesh.IsValid())
		//{
		//	m_Mesh->UploadResources(CommandList);
		//}
		//
		//if (m_OwningSkeletalMeshComponent->IsRenderStateDirty())
		//{
		//	m_Materials.clear();
		//
		//	if (m_Mesh.IsValid())
		//	{
		//		const uint32 MaterialCount = m_Mesh->Data.Materials.size();
		//		const uint32 OverrideMaterialCount = m_OwningSkeletalMeshComponent->m_OverrideMaterials.size();
		//		m_Materials.resize(MaterialCount);
		//
		//		for (int i = 0; i < MaterialCount; i++)
		//		{
		//			if (i < OverrideMaterialCount && m_OwningSkeletalMeshComponent->m_OverrideMaterials[i].m_Overriden)
		//			{
		//				m_Materials[i] = m_OwningSkeletalMeshComponent->m_OverrideMaterials[i];
		//			}
		//			else
		//			{
		//				m_Materials[i] = m_Mesh->Data.Materials[i];
		//			}
		//
		//			// TODO: mark this only in with editor builds
		//			m_Materials[i].LoadChecked();
		//			if (!m_Materials[i].IsValid())
		//			{
		//				LOG(LogSkeletalMeshSceneProxy, Error, "Material is invalid. Using default material.");
		//
		//				m_Materials[i] = AssetHandle<Material>(DEFAULT_MATERIAL_PATH);
		//				m_Materials[i].Load();
		//			}
		//		}
		//	}
		//
		//	bWasDirty = true;
		//	UpdatePrimitiveBuffer(CommandList);
		//}
		//
		//// @HACK: just one frame delay to propagate PrevLocalToWorld
		//if (!m_OwningSkeletalMeshComponent->IsRenderStateDirty() && bWasDirty)
		//{
		//	bWasDirty = false;
		//	UpdatePrimitiveBuffer(CommandList);
		//}
		//
		//for (MaterialSlot& MatSlot : m_Materials)
		//{
		//	MatSlot.GetMaterialInterface()->UploadResources(CommandList);
		//}
		//
		//m_OwningSkeletalMeshComponent->ClearRenderStateDirty();
	}

	void SkeletalMeshSceneProxy::UpdatePrimitiveBuffer(D3D12CommandList* CommandList)
	{
		//// TODO: issue when proxy not begin rendered
		//m_PrimitiveData.m_PrevLocalToWorld = m_PrimitiveData.m_LocalToWorld;
		//m_PrimitiveData.m_LocalToWorld = Matrix(m_OwningSkeletalMeshComponent->GetWorldTransform()).Get();
		//m_PrimitiveData.m_HitProxyData = m_HitProxyData;
		//
		//PrimitiveBuffer = RenderUniformBuffer::Create(CommandList->GetParentDevice(), sizeof(PrimitiveData), EUniformBufferUsage::MultiFrame, &m_PrimitiveData);
	}

	void SkeletalMeshSceneProxy::RenderVelocityPass( class D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		//if (m_Mesh.IsValid())
		//{
		//	const std::string MeshName = Path::GetCleanName(m_Mesh.GetPath());
		//	SCOPE_STAT_DYNAMIC(MeshName.c_str());
		//
		//	for (size_t i = 0; i < m_Mesh->Data.MeshesData.size(); i++)
		//	{
		//		const SkeletalMeshSlotData& RenderProxy = m_Mesh->Data.MeshesData[i];
		//		MaterialSlot& Mat = m_Materials[RenderProxy.MaterialIndex];
		//		
		//		MaterialShader* MatShader = Mat.GetParentMaterial()->GetShaderParameters().bIsUsedWithSkeletalMesh && Mat.GetParentMaterial()->GetShaderParameters().bHasVelocityPass
		//			? Mat.GetParentMaterial()->GetShaders().GetShader(VertexFactoryType::SkeletalMesh, EMaterialStage::Velocity)
		//			: nullptr;
		//
		//		if (MatShader)
		//		{
		//			SCOPE_STAT_DYNAMIC(Mat.GetMaterialName().c_str());
		//
		//			MatShader->Bind(CommandList);
		//			Mat.GetMaterialInterface()->BindResources(CommandList);
		//
		//			CommandList->SetGraphicRootConstant(Renderer->ViewBuffer->GetViewIndex(), 0);
		//			CommandList->SetGraphicRootConstant(PrimitiveBuffer->GetViewIndex(), 1);
		//			CommandList->SetGraphicRootConstant(Renderer::Get()->StaticSamplersBuffer->GetViewIndex(), 2);
		//
		//			RenderProxy.BindAndDraw(CommandList);
		//		}
		//	}
		//}
	}

	void SkeletalMeshSceneProxy::RenderTranslucencyPass( class D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		//if (m_Mesh.IsValid())
		//{
		//	const std::string MeshName = Path::GetCleanName(m_Mesh.GetPath());
		//	SCOPE_STAT_DYNAMIC(MeshName.c_str());
		//
		//	for (size_t i = 0; i < m_Mesh->Data.MeshesData.size(); i++)
		//	{
		//		const SkeletalMeshSlotData& RenderProxy = m_Mesh->Data.MeshesData[i];
		//		MaterialSlot& Mat = m_Materials[RenderProxy.MaterialIndex];
		//		
		//		MaterialShader* MatShader = Mat.GetParentMaterial()->GetShaderParameters().bIsUsedWithSkeletalMesh && Mat.GetParentMaterial()->GetShaderParameters().bHasTranslucencyPass
		//			? Mat.GetParentMaterial()->GetShaders().GetShader(VertexFactoryType::SkeletalMesh, EMaterialStage::Translucensy)
		//			: nullptr;
		//
		//		if (MatShader)
		//		{
		//			SCOPE_STAT_DYNAMIC(Mat.GetMaterialName().c_str());
		//
		//			MatShader->Bind(CommandList);
		//			Mat.GetMaterialInterface()->BindResources(CommandList);
		//
		//			CommandList->SetGraphicRootConstant(Renderer->ViewBuffer->GetViewIndex(), 0);
		//			CommandList->SetGraphicRootConstant(PrimitiveBuffer->GetViewIndex(), 1);
		//			CommandList->SetGraphicRootConstant(Renderer::Get()->StaticSamplersBuffer->GetViewIndex(), 2);
		//
		//			RenderProxy.BindAndDraw(CommandList);
		//		}
		//	}
		//}
	}

	void SkeletalMeshSceneProxy::RenderDistortionPass( class D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		//if (m_Mesh.IsValid())
		//{
		//	const std::string MeshName = Path::GetCleanName(m_Mesh.GetPath());
		//	SCOPE_STAT_DYNAMIC(MeshName.c_str());
		//
		//	for (size_t i = 0; i < m_Mesh->Data.MeshesData.size(); i++)
		//	{
		//		const SkeletalMeshSlotData& RenderProxy = m_Mesh->Data.MeshesData[i];
		//		MaterialSlot& Mat = m_Materials[RenderProxy.MaterialIndex];
		//		
		//		MaterialShader* MatShader = Mat.GetParentMaterial()->GetShaderParameters().bIsUsedWithSkeletalMesh && Mat.GetParentMaterial()->GetShaderParameters().bHasDistortionPass
		//			? Mat.GetParentMaterial()->GetShaders().GetShader(VertexFactoryType::SkeletalMesh, EMaterialStage::Distortion)
		//			: nullptr;
		//
		//		if (MatShader)
		//		{
		//			SCOPE_STAT_DYNAMIC(Mat.GetMaterialName().c_str());
		//
		//			MatShader->Bind(CommandList);
		//			Mat.GetMaterialInterface()->BindResources(CommandList);
		//
		//			CommandList->SetGraphicRootConstant(Renderer->ViewBuffer->GetViewIndex(), 0);
		//			CommandList->SetGraphicRootConstant(PrimitiveBuffer->GetViewIndex(), 1);
		//			CommandList->SetGraphicRootConstant(Renderer::Get()->StaticSamplersBuffer->GetViewIndex(), 2);
		//
		//			RenderProxy.BindAndDraw(CommandList);
		//		}
		//	}
		//}
	}

	void SkeletalMeshSceneProxy::RenderMainPass( D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		//if (m_Mesh.IsValid())
		//{
		//	const std::string MeshName = Path::GetCleanName(m_Mesh.GetPath());
		//	SCOPE_STAT_DYNAMIC(MeshName.c_str());
		//
		//	for (size_t i = 0; i < m_Mesh->Data.MeshesData.size(); i++)
		//	{
		//		const SkeletalMeshSlotData& RenderProxy = m_Mesh->Data.MeshesData[i];
		//		MaterialSlot& Mat = m_Materials[RenderProxy.MaterialIndex];
		//		
		//		MaterialShader* MatShader = Mat.GetParentMaterial()->GetShaderParameters().bIsUsedWithSkeletalMesh && Mat.GetParentMaterial()->GetShaderParameters().bHasMainPass
		//			? Mat.GetParentMaterial()->GetShaders().GetShader(VertexFactoryType::SkeletalMesh, EMaterialStage::Main)
		//			: nullptr;
		//
		//		if (MatShader)
		//		{
		//			SCOPE_STAT_DYNAMIC(Mat.GetMaterialName().c_str());
		//
		//			MatShader->Bind(CommandList);
		//			Mat.GetMaterialInterface()->BindResources(CommandList);
		//
		//			CommandList->SetGraphicRootConstant(Renderer->ViewBuffer->GetViewIndex(), 0);
		//			CommandList->SetGraphicRootConstant(PrimitiveBuffer->GetViewIndex(), 1);
		//			CommandList->SetGraphicRootConstant(Renderer::Get()->StaticSamplersBuffer->GetViewIndex(), 2);
		//
		//			RenderProxy.BindAndDraw(CommandList);
		//		}
		//	}
		//}
	}

	void SkeletalMeshSceneProxy::RenderPrePass( D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		//if (m_Mesh.IsValid())
		//{
		//	const std::string MeshName = Path::GetCleanName(m_Mesh.GetPath());
		//	SCOPE_STAT_DYNAMIC(MeshName.c_str());
		//
		//	for (size_t i = 0; i < m_Mesh->Data.MeshesData.size(); i++)
		//	{
		//		const SkeletalMeshSlotData& RenderProxy = m_Mesh->Data.MeshesData[i];
		//		MaterialSlot& Mat = m_Materials[RenderProxy.MaterialIndex];
		//
		//		MaterialShader* MatShader = nullptr;
		//		if (Mat.GetParentMaterial()->GetShaderParameters().bIsUsedWithSkeletalMesh && Mat.GetParentMaterial()->GetShaderParameters().bHasPrepass)
		//		{
		//			MatShader = Mat.GetParentMaterial()->GetShaderParameters().bHasCustomPrepass
		//				? Mat.GetParentMaterial()->GetShaders().GetShader(VertexFactoryType::SkeletalMesh, EMaterialStage::Prepass)
		//				: CommonResources::Get()->m_PositionOnlyMaterialShaders.GetShader(VertexFactoryType::SkeletalMesh, Mat.GetParentMaterial()->IsTwoSided());
		//		}
		//
		//		if (MatShader)
		//		{
		//			SCOPE_STAT_DYNAMIC(Mat.GetMaterialName().c_str());
		//
		//			MatShader->Bind(CommandList);
		//			Mat.GetMaterialInterface()->BindResources(CommandList);
		//
		//			CommandList->SetGraphicRootConstant(Renderer->ViewBuffer->GetViewIndex(), 0);
		//			CommandList->SetGraphicRootConstant(PrimitiveBuffer->GetViewIndex(), 1);
		//			CommandList->SetGraphicRootConstant(Renderer::Get()->StaticSamplersBuffer->GetViewIndex(), 2);
		//
		//			RenderProxy.BindAndDraw(CommandList);
		//		}
		//	}
		//}
	}

	void SkeletalMeshSceneProxy::RenderShadowPass( D3D12CommandList* CommandList, SceneRenderer* Renderer, LightSceneProxy* LightProxy)
	{
		//auto GetMaterialShaderForLightType = [](const MaterialShaders& Shaders, ELightType Type)
		//{
		//	switch ( Type )
		//	{
		//	case ELightType::PointLight: return Shaders.GetShader(VertexFactoryType::SkeletalMesh, EMaterialStage::PointLightShadow);
		//	case ELightType::SpotLight:
		//	case ELightType::DirectionalLight: return Shaders.GetShader(VertexFactoryType::SkeletalMesh, EMaterialStage::SpotLightShadow);
		//	case ELightType::SkyLight:
		//	default: drn_check(false); return Shaders.GetShader(VertexFactoryType::SkeletalMesh, EMaterialStage::PointLightShadow);
		//	}
		//};
		//
		//if (m_Mesh.IsValid())
		//{
		//	const std::string MeshName = Path::GetCleanName(m_Mesh.GetPath());
		//	SCOPE_STAT_DYNAMIC(MeshName.c_str());
		//
		//	for (size_t i = 0; i < m_Mesh->Data.MeshesData.size(); i++)
		//	{
		//		const SkeletalMeshSlotData& RenderProxy = m_Mesh->Data.MeshesData[i];
		//		MaterialSlot& Mat = m_Materials[RenderProxy.MaterialIndex];
		//
		//		MaterialShader* MatShader = Mat.GetParentMaterial()->GetShaderParameters().bIsUsedWithSkeletalMesh && Mat.GetParentMaterial()->GetShaderParameters().bHasShadowPass
		//			? GetMaterialShaderForLightType(Mat.GetParentMaterial()->GetShaders(), LightProxy->GetLightType())
		//			: nullptr;
		//
		//		if (MatShader)
		//		{
		//			SCOPE_STAT_DYNAMIC(Mat.GetMaterialName().c_str());
		//
		//			MatShader->Bind(CommandList);
		//			Mat.GetMaterialInterface()->BindResources(CommandList);
		//
		//			CommandList->SetGraphicRootConstant(Renderer->ViewBuffer->GetViewIndex(), 0);
		//			CommandList->SetGraphicRootConstant(PrimitiveBuffer->GetViewIndex(), 1);
		//			CommandList->SetGraphicRootConstant(Renderer::Get()->StaticSamplersBuffer->GetViewIndex(), 2);
		//
		//			RenderProxy.BindAndDraw(CommandList);
		//		}
		//
		//	}
		//}
	}

	void SkeletalMeshSceneProxy::RenderDecalPass( D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		//if (m_Mesh.IsValid())
		//{
		//	const std::string MeshName = Path::GetCleanName(m_Mesh.GetPath());
		//	SCOPE_STAT_DYNAMIC(MeshName.c_str());
		//
		//	for (size_t i = 0; i < m_Mesh->Data.MeshesData.size(); i++)
		//	{
		//		const SkeletalMeshSlotData& RenderProxy = m_Mesh->Data.MeshesData[i];
		//		MaterialSlot& Mat = m_Materials[RenderProxy.MaterialIndex];
		//
		//		MaterialShader* MatShader = Mat.GetParentMaterial()->GetShaderParameters().bIsUsedWithSkeletalMesh && Mat.GetParentMaterial()->GetShaderParameters().bHasDecalPass
		//			? Mat.GetParentMaterial()->GetShaders().GetShader(VertexFactoryType::SkeletalMesh, EMaterialStage::Decal)
		//			: nullptr;
		//
		//		if (MatShader)
		//		{
		//			SCOPE_STAT_DYNAMIC(Mat.GetMaterialName().c_str());
		//
		//			MatShader->Bind(CommandList);
		//			Mat.GetMaterialInterface()->BindResources(CommandList);
		//
		//			CommandList->SetGraphicRootConstant(Renderer->ViewBuffer->GetViewIndex(), 0);
		//			CommandList->SetGraphicRootConstant(PrimitiveBuffer->GetViewIndex(), 1);
		//			CommandList->SetGraphicRootConstant(Renderer::Get()->StaticSamplersBuffer->GetViewIndex(), 2);
		//
		//			RenderProxy.BindAndDraw(CommandList);
		//		}
		//	}
		//}
	}

#if WITH_EDITOR

	void SkeletalMeshSceneProxy::RenderHitProxyPass( D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		//SCOPE_STAT("HitProxyMesh");
		//
		//if (!m_Mesh.IsValid() || !m_Selectable)
		//{
		//	return;
		//}
		//
		//for (size_t i = 0; i < m_Mesh->Data.MeshesData.size(); i++)
		//{
		//	const SkeletalMeshSlotData& RenderProxy = m_Mesh->Data.MeshesData[i];
		//	MaterialSlot& Mat = m_Materials[RenderProxy.MaterialIndex];
		//
		//	MaterialShader* MatShader = Mat.GetParentMaterial()->GetShaderParameters().bIsUsedWithSkeletalMesh && Mat.GetParentMaterial()->GetShaderParameters().bHasHitProxyPass
		//		? Mat.GetParentMaterial()->GetShaders().GetShader(VertexFactoryType::SkeletalMesh, EMaterialStage::Hitproxy)
		//		: nullptr;
		//
		//	if (MatShader)
		//	{
		//		SCOPE_STAT_DYNAMIC(Mat.GetMaterialName().c_str());
		//
		//		MatShader->Bind(CommandList);
		//		Mat.GetMaterialInterface()->BindResources(CommandList);
		//
		//		CommandList->SetGraphicRootConstant(Renderer->ViewBuffer->GetViewIndex(), 0);
		//		CommandList->SetGraphicRootConstant(PrimitiveBuffer->GetViewIndex(), 1);
		//		CommandList->SetGraphicRootConstant(Renderer::Get()->StaticSamplersBuffer->GetViewIndex(), 2);
		//
		//		RenderProxy.BindAndDraw(CommandList);
		//	}
		//}
	}

	void SkeletalMeshSceneProxy::RenderSelectionPass( D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		//if (!m_Mesh.IsValid() || !m_SelectedInEditor)
		//	return;
		//
		//const std::string MeshName = Path::GetCleanName(m_Mesh.GetPath());
		//SCOPE_STAT_DYNAMIC(MeshName.c_str());
		//
		//for (size_t i = 0; i < m_Mesh->Data.MeshesData.size(); i++)
		//{
		//	const SkeletalMeshSlotData& RenderProxy = m_Mesh->Data.MeshesData[i];
		//	MaterialSlot& Mat = m_Materials[RenderProxy.MaterialIndex];
		//
		//	MaterialShader* MatShader = Mat.GetParentMaterial()->GetShaderParameters().bIsUsedWithSkeletalMesh && Mat.GetParentMaterial()->GetShaderParameters().bHasEditorSelectionPass
		//		? Mat.GetParentMaterial()->GetShaders().GetShader(VertexFactoryType::SkeletalMesh, EMaterialStage::EditorSelection)
		//		: nullptr;
		//	if (MatShader)
		//	{
		//		SCOPE_STAT_DYNAMIC(Mat.GetMaterialName().c_str());
		//
		//		MatShader->Bind(CommandList);
		//		Mat.GetMaterialInterface()->BindResources(CommandList);
		//
		//		CommandList->SetGraphicRootConstant(Renderer->ViewBuffer->GetViewIndex(), 0);
		//		CommandList->SetGraphicRootConstant(PrimitiveBuffer->GetViewIndex(), 1);
		//		CommandList->SetGraphicRootConstant(Renderer::Get()->StaticSamplersBuffer->GetViewIndex(), 2);
		//
		//		RenderProxy.BindAndDraw(CommandList);
		//	}
		//}
	}

	void SkeletalMeshSceneProxy::RenderEditorPrimitivePass( D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		//if (!m_Mesh.IsValid() || !m_EditorPrimitive)
		//{
		//	return;
		//}
		//
		//const std::string MeshName = Path::GetCleanName(m_Mesh.GetPath());
		//SCOPE_STAT_DYNAMIC(MeshName.c_str());
		//
		//for (size_t i = 0; i < m_Mesh->Data.MeshesData.size(); i++)
		//{
		//	const SkeletalMeshSlotData& RenderProxy = m_Mesh->Data.MeshesData[i];
		//	MaterialSlot& Mat = m_Materials[RenderProxy.MaterialIndex];
		//
		//	MaterialShader* MatShader = Mat.GetParentMaterial()->GetShaderParameters().bIsUsedWithSkeletalMesh && Mat.GetParentMaterial()->GetShaderParameters().bHasEditorPrimitivePass
		//		? Mat.GetParentMaterial()->GetShaders().GetShader(VertexFactoryType::SkeletalMesh, EMaterialStage::EditorPrimitive)
		//		: nullptr;
		//	if (MatShader)
		//	{
		//		SCOPE_STAT_DYNAMIC(Mat.GetMaterialName().c_str());
		//
		//		MatShader->Bind(CommandList);
		//		Mat.GetMaterialInterface()->BindResources(CommandList);
		//
		//		CommandList->SetGraphicRootConstant(Renderer->ViewBuffer->GetViewIndex(), 0);
		//		CommandList->SetGraphicRootConstant(PrimitiveBuffer->GetViewIndex(), 1);
		//		CommandList->SetGraphicRootConstant(Renderer::Get()->StaticSamplersBuffer->GetViewIndex(), 2);
		//
		//		RenderProxy.BindAndDraw( CommandList );
		//	}
		//
		//}
	}

#endif

}