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
		//		if (!Mat.GetParentMaterial()->HasBasePass())
		//		{
		//			continue;
		//		}
		//
		//		SCOPE_STAT_DYNAMIC(Mat.GetMaterialName().c_str());
		//
		//		Mat.GetParentMaterial()->BindMainPass( CommandList );
		//		Mat.GetMaterialInterface()->BindResources(CommandList);
		//
		//		m_PrimitiveBuffer.m_LocalToWorld = Matrix(m_OwningInstancedStaticMeshComponent->GetWorldTransform()).Get();
		//		m_PrimitiveBuffer.m_LocalToProjection = XMMatrixMultiply( m_PrimitiveBuffer.m_LocalToWorld.Get(), Renderer->GetSceneView().WorldToProjection.Get() );
		//		m_PrimitiveBuffer.m_Guid = m_Guid;
		//
		//		// TODO: cache in different draws
		//		TRefCountPtr<RenderUniformBuffer> MeshBuffer = RenderUniformBuffer::Create(CommandList->GetParentDevice(), sizeof(PrimitiveBuffer), EUniformBufferUsage::SingleFrame, &m_PrimitiveBuffer);
		//
		//		CommandList->SetGraphicRootConstant(Renderer->ViewBuffer->GetViewIndex(), 0);
		//		CommandList->SetGraphicRootConstant(MeshBuffer->GetViewIndex(), 1);
		//		CommandList->SetGraphicRootConstant(Renderer::Get()->StaticSamplersBuffer->GetViewIndex(), 2);
		//
		//		RenderProxy.BindAndDraw(CommandList);
		//	}
		//}

		if (m_Mesh.IsValid())
		{
			const std::string MeshName = Path::GetCleanName(m_Mesh.GetPath());
			SCOPE_STAT_DYNAMIC(MeshName.c_str());
		
			for (size_t i = 0; i < m_Mesh->Data.MeshesData.size(); i++)
			{
				const int32 NumInstances = m_OwningInstancedStaticMeshComponent->GetInstanceCount();
				if (NumInstances == 0)
				{
					continue;
				}

				const StaticMeshSlotData& RenderProxy = m_Mesh->Data.MeshesData[i];
				MaterialSlot& Mat = m_Materials[RenderProxy.MaterialIndex];
				
				MaterialShader* MatShader = Mat.GetParentMaterial()->GetShaderParameters().bIsUsedWithInstancedStaticMesh && Mat.GetParentMaterial()->GetShaderParameters().bHasMainPass
					? Mat.GetParentMaterial()->GetShaders().GetShader(VertexFactoryType::InstancedStaticMesh, EMaterialStage::Main)
					: nullptr;

				if (MatShader)
				{
					SCOPE_STAT_DYNAMIC(Mat.GetMaterialName().c_str());
		
					MatShader->Bind(CommandList);
					Mat.GetMaterialInterface()->BindResources(CommandList);
		
					m_PrimitiveBuffer.m_LocalToWorld = Matrix(m_OwningInstancedStaticMeshComponent->GetWorldTransform()).Get();
					m_PrimitiveBuffer.m_LocalToProjection = XMMatrixMultiply( m_PrimitiveBuffer.m_LocalToWorld.Get(), Renderer->GetSceneView().WorldToProjection.Get() );
					m_PrimitiveBuffer.m_Guid = m_Guid;
		
					// TODO: cache in different draws
					TRefCountPtr<RenderUniformBuffer> MeshBuffer = RenderUniformBuffer::Create(CommandList->GetParentDevice(), sizeof(PrimitiveBuffer), EUniformBufferUsage::SingleFrame, &m_PrimitiveBuffer);
		
					CommandList->SetGraphicRootConstant(Renderer->ViewBuffer->GetViewIndex(), 0);
					CommandList->SetGraphicRootConstant(MeshBuffer->GetViewIndex(), 1);
					CommandList->SetGraphicRootConstant(Renderer::Get()->StaticSamplersBuffer->GetViewIndex(), 2);
		
					//RenderProxy.BindAndDraw(CommandList);

					uint32 VertexCount = RenderProxy.VertexData.GetVertexCount();
					uint32 PrimitiveCount = RenderProxy.VertexData.GetPrimitiveCount();
					RenderProxy.m_StaticMeshVertexBuffer->Bind(CommandList);

					auto CreateVertexBufferConditional = [](bool bCreate, D3D12CommandList* CmdList, TRefCountPtr<class RenderVertexBuffer>& Buffer, void* Data, uint32 Size, const std::string& Name)
					{
						if (bCreate)
						{
							uint32 VertexBufferFlags = (uint32)EBufferUsageFlags::VertexBuffer | (uint32)EBufferUsageFlags::Static;
							RenderResourceCreateInfo VertexBufferCreateInfo(nullptr, Data, ClearValueBinding::Black, Name);
							Buffer = RenderVertexBuffer::Create(CmdList->GetParentDevice(), CmdList, Size, VertexBufferFlags, D3D12_RESOURCE_STATE_COMMON, false, VertexBufferCreateInfo);
						}
						else
						{
							Buffer = nullptr;
						}
					};

					TRefCountPtr<RenderVertexBuffer> OriginRandomBuffer;
					TRefCountPtr<RenderVertexBuffer> LocalToWorldBuffer;

					std::vector<Vector4> OriginRandom;
					OriginRandom.resize(NumInstances);

					struct LocalToWorldMat
					{
						LocalToWorldMat()
						{}

						LocalToWorldMat(const Matrix InMatrix)
							: m11(InMatrix.m_Matrix._11), m12(InMatrix.m_Matrix._12), m13(InMatrix.m_Matrix._13), m14(InMatrix.m_Matrix._14)
							, m21(InMatrix.m_Matrix._21), m22(InMatrix.m_Matrix._22), m23(InMatrix.m_Matrix._23), m24(InMatrix.m_Matrix._24)
							, m31(InMatrix.m_Matrix._31), m32(InMatrix.m_Matrix._32), m33(InMatrix.m_Matrix._33), m34(InMatrix.m_Matrix._34)
						{}

						Float16 m11, m12, m13, m14;
						Float16 m21, m22, m23, m24;
						Float16 m31, m32, m33, m34;
					};

					std::vector<LocalToWorldMat> LocalToWorld;
					LocalToWorld.resize(NumInstances);

					for (int32 InstanceIndex = 0; InstanceIndex < NumInstances; InstanceIndex++)
					{
						Transform LocalToWorldTransform;
						m_OwningInstancedStaticMeshComponent->GetInstanceTransform(InstanceIndex, LocalToWorldTransform, true);

						LocalToWorld[InstanceIndex] = LocalToWorldMat(LocalToWorldTransform);
						OriginRandom[InstanceIndex] = Vector4(LocalToWorldTransform.GetLocation(), 0);
					}

					CreateVertexBufferConditional(true, CommandList, OriginRandomBuffer, (void*)OriginRandom.data(), NumInstances * sizeof(Vector4), "VB_PositionRandom");
					CreateVertexBufferConditional(true, CommandList, LocalToWorldBuffer, (void*)LocalToWorld.data(), NumInstances * sizeof(LocalToWorld), "VB_LocalToWorld");

					CommandList->SetStreamSource(8, OriginRandomBuffer, 0);
					CommandList->SetStreamSource(9, LocalToWorldBuffer, 0);

					CommandList->DrawIndexedPrimitive(RenderProxy.m_IndexBuffer, 0, 0, VertexCount, 0, PrimitiveCount, NumInstances);
				}
		
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
				const int32 NumInstances = m_OwningInstancedStaticMeshComponent->GetInstanceCount();
				if (NumInstances == 0)
				{
					continue;
				}

				const StaticMeshSlotData& RenderProxy = m_Mesh->Data.MeshesData[i];
				MaterialSlot& Mat = m_Materials[RenderProxy.MaterialIndex];

				MaterialShader* MatShader = nullptr;
				if (Mat.GetParentMaterial()->GetShaderParameters().bIsUsedWithInstancedStaticMesh && Mat.GetParentMaterial()->GetShaderParameters().bHasPrepass)
				{
					MatShader = Mat.GetParentMaterial()->GetShaderParameters().bHasCustomPrepass
						? Mat.GetParentMaterial()->GetShaders().GetShader(VertexFactoryType::InstancedStaticMesh, EMaterialStage::Prepass)
						: CommonResources::Get()->m_PositionOnlyMaterialShaders.GetShader(VertexFactoryType::InstancedStaticMesh, Mat.GetParentMaterial()->IsTwoSided());
				}

				if (MatShader)
				{
					SCOPE_STAT_DYNAMIC(Mat.GetMaterialName().c_str());
		
					MatShader->Bind(CommandList);
					Mat.GetMaterialInterface()->BindResources(CommandList);
		
					m_PrimitiveBuffer.m_LocalToWorld = Matrix(m_OwningInstancedStaticMeshComponent->GetWorldTransform()).Get();
					m_PrimitiveBuffer.m_LocalToProjection = XMMatrixMultiply( m_PrimitiveBuffer.m_LocalToWorld.Get(), Renderer->GetSceneView().WorldToProjection.Get() );
					m_PrimitiveBuffer.m_Guid = m_Guid;
		
					TRefCountPtr<RenderUniformBuffer> MeshBuffer = RenderUniformBuffer::Create(CommandList->GetParentDevice(), sizeof(PrimitiveBuffer), EUniformBufferUsage::SingleFrame, &m_PrimitiveBuffer);
		
					CommandList->SetGraphicRootConstant(Renderer->ViewBuffer->GetViewIndex(), 0);
					CommandList->SetGraphicRootConstant(MeshBuffer->GetViewIndex(), 1);
					CommandList->SetGraphicRootConstant(Renderer::Get()->StaticSamplersBuffer->GetViewIndex(), 2);
		
					//RenderProxy.BindAndDraw(CommandList);

					uint32 VertexCount = RenderProxy.VertexData.GetVertexCount();
					uint32 PrimitiveCount = RenderProxy.VertexData.GetPrimitiveCount();
					RenderProxy.m_StaticMeshVertexBuffer->Bind(CommandList);

					auto CreateVertexBufferConditional = [](bool bCreate, D3D12CommandList* CmdList, TRefCountPtr<class RenderVertexBuffer>& Buffer, void* Data, uint32 Size, const std::string& Name)
					{
						if (bCreate)
						{
							uint32 VertexBufferFlags = (uint32)EBufferUsageFlags::VertexBuffer | (uint32)EBufferUsageFlags::Static;
							RenderResourceCreateInfo VertexBufferCreateInfo(nullptr, Data, ClearValueBinding::Black, Name);
							Buffer = RenderVertexBuffer::Create(CmdList->GetParentDevice(), CmdList, Size, VertexBufferFlags, D3D12_RESOURCE_STATE_COMMON, false, VertexBufferCreateInfo);
						}
						else
						{
							Buffer = nullptr;
						}
					};

					TRefCountPtr<RenderVertexBuffer> OriginRandomBuffer;
					TRefCountPtr<RenderVertexBuffer> LocalToWorldBuffer;

					std::vector<Vector4> OriginRandom;
					OriginRandom.resize(NumInstances);

					struct LocalToWorldMat
					{
						LocalToWorldMat()
						{}

						LocalToWorldMat(const Matrix InMatrix)
							: m11(InMatrix.m_Matrix._11), m12(InMatrix.m_Matrix._12), m13(InMatrix.m_Matrix._13), m14(InMatrix.m_Matrix._14)
							, m21(InMatrix.m_Matrix._21), m22(InMatrix.m_Matrix._22), m23(InMatrix.m_Matrix._23), m24(InMatrix.m_Matrix._24)
							, m31(InMatrix.m_Matrix._31), m32(InMatrix.m_Matrix._32), m33(InMatrix.m_Matrix._33), m34(InMatrix.m_Matrix._34)
						{}

						Float16 m11, m12, m13, m14;
						Float16 m21, m22, m23, m24;
						Float16 m31, m32, m33, m34;
					};

					std::vector<LocalToWorldMat> LocalToWorld;
					LocalToWorld.resize(NumInstances);

					for (int32 InstanceIndex = 0; InstanceIndex < NumInstances; InstanceIndex++)
					{
						Transform LocalToWorldTransform;
						m_OwningInstancedStaticMeshComponent->GetInstanceTransform(InstanceIndex, LocalToWorldTransform, true);

						LocalToWorld[InstanceIndex] = LocalToWorldMat(LocalToWorldTransform);
						OriginRandom[InstanceIndex] = Vector4(LocalToWorldTransform.GetLocation(), 0);
					}

					CreateVertexBufferConditional(true, CommandList, OriginRandomBuffer, (void*)OriginRandom.data(), NumInstances * sizeof(Vector4), "VB_PositionRandom");
					CreateVertexBufferConditional(true, CommandList, LocalToWorldBuffer, (void*)LocalToWorld.data(), NumInstances * sizeof(LocalToWorld), "VB_LocalToWorld");

					CommandList->SetStreamSource(8, OriginRandomBuffer, 0);
					CommandList->SetStreamSource(9, LocalToWorldBuffer, 0);

					CommandList->DrawIndexedPrimitive(RenderProxy.m_IndexBuffer, 0, 0, VertexCount, 0, PrimitiveCount, NumInstances);
				}
			}
		}
	}

	void InstancedStaticMeshSceneProxy::RenderShadowPass( class D3D12CommandList* CommandList, SceneRenderer* Renderer, LightSceneProxy* LightProxy )
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
		//		if (!Mat.GetParentMaterial()->HasShadowPass())
		//		{
		//			continue;
		//		}
		//
		//		SCOPE_STAT_DYNAMIC(Mat.GetMaterialName().c_str());
		//
		//		if ( LightProxy->GetLightType() == ELightType::PointLight )
		//		{
		//			Mat.GetParentMaterial()->BindPointLightShadowDepthPass(CommandList);
		//		}
		//		else if ( LightProxy->GetLightType() == ELightType::SpotLight)
		//		{
		//			Mat.GetParentMaterial()->BindSpotLightShadowDepthPass(CommandList);
		//		}
		//		else if ( LightProxy->GetLightType() == ELightType::DirectionalLight)
		//		{
		//			Mat.GetParentMaterial()->BindSpotLightShadowDepthPass(CommandList);
		//		}
		//
		//		Mat.GetMaterialInterface()->BindResources(CommandList);
		//
		//		m_PrimitiveBuffer.m_LocalToWorld = Matrix(m_OwningInstancedStaticMeshComponent->GetWorldTransform()).Get();
		//		m_PrimitiveBuffer.m_LocalToProjection = XMMatrixMultiply( m_PrimitiveBuffer.m_LocalToWorld.Get(), Renderer->GetSceneView().WorldToProjection.Get() );
		//		m_PrimitiveBuffer.m_Guid = m_Guid;
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
		//SCOPE_STAT("HitProxyMesh");
		//
		//if (!m_Mesh.IsValid() || !m_Selectable)
		//{
		//	return;
		//}
		//
		//for (size_t i = 0; i < m_Mesh->Data.MeshesData.size(); i++)
		//{
		//	const StaticMeshSlotData& RenderProxy = m_Mesh->Data.MeshesData[i];
		//	MaterialSlot& Mat = m_Materials[RenderProxy.MaterialIndex];
		//
		//
		//	if (!Mat.GetParentMaterial()->HasHitProxyPass())
		//	{
		//		continue;
		//	}
		//
		//	SCOPE_STAT_DYNAMIC(Mat.GetMaterialName().c_str());
		//
		//	Mat.GetParentMaterial()->BindHitProxyPass(CommandList);
		//	Mat.GetMaterialInterface()->BindResources(CommandList);
		//
		//	{
		//		SCOPE_STAT("Calculate");
		//
		//		m_PrimitiveBuffer.m_LocalToWorld = Matrix(m_OwningInstancedStaticMeshComponent->GetWorldTransform()).Get();
		//		m_PrimitiveBuffer.m_LocalToProjection = XMMatrixMultiply( m_PrimitiveBuffer.m_LocalToWorld.Get(), Renderer->GetSceneView().WorldToProjection.Get() );
		//		m_PrimitiveBuffer.m_Guid = m_Guid;
		//	}
		//
		//	TRefCountPtr<RenderUniformBuffer> MeshBuffer = RenderUniformBuffer::Create(CommandList->GetParentDevice(), sizeof(PrimitiveBuffer), EUniformBufferUsage::SingleFrame, &m_PrimitiveBuffer);
		//
		//	CommandList->SetGraphicRootConstant(Renderer->ViewBuffer->GetViewIndex(), 0);
		//	CommandList->SetGraphicRootConstant(MeshBuffer->GetViewIndex(), 1);
		//	CommandList->SetGraphicRootConstant(Renderer::Get()->StaticSamplersBuffer->GetViewIndex(), 2);
		//
		//	RenderProxy.BindAndDraw(CommandList);
		//}
	}

	void InstancedStaticMeshSceneProxy::RenderSelectionPass( class D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		//if (!m_Mesh.IsValid() || !m_SelectedInEditor)
		//	return;
		//
		//const std::string MeshName = Path::GetCleanName(m_Mesh.GetPath());
		//SCOPE_STAT_DYNAMIC(MeshName.c_str());
		//
		//for (size_t i = 0; i < m_Mesh->Data.MeshesData.size(); i++)
		//{
		//	const StaticMeshSlotData& RenderProxy = m_Mesh->Data.MeshesData[i];
		//	MaterialSlot& Mat = m_Materials[RenderProxy.MaterialIndex];
		//
		//	if (!Mat.GetParentMaterial()->HasEditorSelectionPass())
		//	{
		//		continue;
		//	}
		//
		//	SCOPE_STAT_DYNAMIC(Mat.GetMaterialName().c_str());
		//
		//	Mat.GetParentMaterial()->BindSelectionPass(CommandList);
		//	Mat.GetMaterialInterface()->BindResources(CommandList);
		//
		//	m_PrimitiveBuffer.m_LocalToWorld = Matrix(m_OwningInstancedStaticMeshComponent->GetWorldTransform()).Get();
		//	m_PrimitiveBuffer.m_LocalToProjection = XMMatrixMultiply( m_PrimitiveBuffer.m_LocalToWorld.Get(), Renderer->GetSceneView().WorldToProjection.Get() );
		//	m_PrimitiveBuffer.m_Guid = m_Guid;
		//
		//	TRefCountPtr<RenderUniformBuffer> MeshBuffer = RenderUniformBuffer::Create(CommandList->GetParentDevice(), sizeof(PrimitiveBuffer), EUniformBufferUsage::SingleFrame, &m_PrimitiveBuffer);
		//
		//	CommandList->SetGraphicRootConstant(Renderer->ViewBuffer->GetViewIndex(), 0);
		//	CommandList->SetGraphicRootConstant(MeshBuffer->GetViewIndex(), 1);
		//	CommandList->SetGraphicRootConstant(Renderer::Get()->StaticSamplersBuffer->GetViewIndex(), 2);
		//
		//	RenderProxy.BindAndDraw(CommandList);
		//}
	}

	void InstancedStaticMeshSceneProxy::RenderEditorPrimitivePass( class D3D12CommandList* CommandList, SceneRenderer* Renderer )
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
		//	const StaticMeshSlotData& RenderProxy = m_Mesh->Data.MeshesData[i];
		//	MaterialSlot& Mat = m_Materials[RenderProxy.MaterialIndex];
		//
		//	if (!Mat.GetParentMaterial()->HasEditorPrimitivePass())
		//	{
		//		continue;
		//	}
		//
		//	SCOPE_STAT_DYNAMIC(Mat.GetMaterialName().c_str());
		//
		//	Mat.GetParentMaterial()->BindEditorPrimitivePass(CommandList);
		//	Mat.GetMaterialInterface()->BindResources(CommandList);
		//
		//	m_PrimitiveBuffer.m_LocalToWorld = Matrix(m_OwningInstancedStaticMeshComponent->GetWorldTransform()).Get();
		//	m_PrimitiveBuffer.m_LocalToProjection = XMMatrixMultiply( m_PrimitiveBuffer.m_LocalToWorld.Get(), Renderer->GetSceneView().WorldToProjection.Get() );
		//	m_PrimitiveBuffer.m_Guid = m_Guid;
		//
		//	TRefCountPtr<RenderUniformBuffer> MeshBuffer = RenderUniformBuffer::Create(CommandList->GetParentDevice(), sizeof(PrimitiveBuffer), EUniformBufferUsage::SingleFrame, &m_PrimitiveBuffer);
		//
		//	CommandList->SetGraphicRootConstant(Renderer->ViewBuffer->GetViewIndex(), 0);
		//	CommandList->SetGraphicRootConstant(MeshBuffer->GetViewIndex(), 1);
		//	CommandList->SetGraphicRootConstant(Renderer::Get()->StaticSamplersBuffer->GetViewIndex(), 2);
		//
		//	RenderProxy.BindAndDraw( CommandList );
		//}
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