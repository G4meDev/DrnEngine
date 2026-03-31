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

	void InstancedStaticMeshSceneProxy::RenderVelocityPass( class D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		
	}

	void InstancedStaticMeshSceneProxy::RenderTranslucencyPass( class D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		const int32 NumInstances = m_OwningInstancedStaticMeshComponent->GetInstanceCount();
		if (NumInstances > 0 && m_Mesh.IsValid())
		{
			const std::string MeshName = Path::GetCleanName(m_Mesh.GetPath());
			SCOPE_STAT_DYNAMIC(MeshName.c_str());
		
			for (size_t i = 0; i < m_Mesh->Data.MeshesData.size(); i++)
			{
				const StaticMeshSlotData& RenderProxy = m_Mesh->Data.MeshesData[i];
				MaterialSlot& Mat = m_Materials[RenderProxy.MaterialIndex];
				
				MaterialShader* MatShader = Mat.GetParentMaterial()->GetShaderParameters().bIsUsedWithInstancedStaticMesh && Mat.GetParentMaterial()->GetShaderParameters().bHasTranslucencyPass
					? Mat.GetParentMaterial()->GetShaders().GetShader(VertexFactoryType::InstancedStaticMesh, EMaterialStage::Translucensy)
					: nullptr;

				if (MatShader)
				{
					SCOPE_STAT_DYNAMIC(Mat.GetMaterialName().c_str());
		
					MatShader->Bind(CommandList);
					Mat.GetMaterialInterface()->BindResources(CommandList);
		
					CommandList->SetGraphicRootConstant(Renderer->ViewBuffer->GetViewIndex(), 0);
					CommandList->SetGraphicRootConstant(PrimitiveBuffer->GetViewIndex(), 1);
					CommandList->SetGraphicRootConstant(Renderer::Get()->StaticSamplersBuffer->GetViewIndex(), 2);
		
					RenderProxy.m_StaticMeshVertexBuffer->Bind(CommandList);
					BindInstanceBuffers(CommandList);

					uint32 VertexCount = RenderProxy.VertexData.GetVertexCount();
					uint32 PrimitiveCount = RenderProxy.VertexData.GetPrimitiveCount();
					CommandList->DrawIndexedPrimitive(RenderProxy.m_IndexBuffer, 0, 0, VertexCount, 0, PrimitiveCount, NumInstances);
				}
			}
		}
	}

	void InstancedStaticMeshSceneProxy::RenderDistortionPass( class D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		const int32 NumInstances = m_OwningInstancedStaticMeshComponent->GetInstanceCount();
		if (NumInstances > 0 && m_Mesh.IsValid())
		{
			const std::string MeshName = Path::GetCleanName(m_Mesh.GetPath());
			SCOPE_STAT_DYNAMIC(MeshName.c_str());
		
			for (size_t i = 0; i < m_Mesh->Data.MeshesData.size(); i++)
			{
				const StaticMeshSlotData& RenderProxy = m_Mesh->Data.MeshesData[i];
				MaterialSlot& Mat = m_Materials[RenderProxy.MaterialIndex];
				
				MaterialShader* MatShader = Mat.GetParentMaterial()->GetShaderParameters().bIsUsedWithInstancedStaticMesh && Mat.GetParentMaterial()->GetShaderParameters().bHasDistortionPass
					? Mat.GetParentMaterial()->GetShaders().GetShader(VertexFactoryType::InstancedStaticMesh, EMaterialStage::Distortion)
					: nullptr;

				if (MatShader)
				{
					SCOPE_STAT_DYNAMIC(Mat.GetMaterialName().c_str());
		
					MatShader->Bind(CommandList);
					Mat.GetMaterialInterface()->BindResources(CommandList);
		
					CommandList->SetGraphicRootConstant(Renderer->ViewBuffer->GetViewIndex(), 0);
					CommandList->SetGraphicRootConstant(PrimitiveBuffer->GetViewIndex(), 1);
					CommandList->SetGraphicRootConstant(Renderer::Get()->StaticSamplersBuffer->GetViewIndex(), 2);
		
					RenderProxy.m_StaticMeshVertexBuffer->Bind(CommandList);
					BindInstanceBuffers(CommandList);

					uint32 VertexCount = RenderProxy.VertexData.GetVertexCount();
					uint32 PrimitiveCount = RenderProxy.VertexData.GetPrimitiveCount();
					CommandList->DrawIndexedPrimitive(RenderProxy.m_IndexBuffer, 0, 0, VertexCount, 0, PrimitiveCount, NumInstances);
				}
			}
		}
	}

	void InstancedStaticMeshSceneProxy::RenderMainPass( class D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		const int32 NumInstances = m_OwningInstancedStaticMeshComponent->GetInstanceCount();
		if (NumInstances > 0 && m_Mesh.IsValid())
		{
			const std::string MeshName = Path::GetCleanName(m_Mesh.GetPath());
			SCOPE_STAT_DYNAMIC(MeshName.c_str());
		
			for (size_t i = 0; i < m_Mesh->Data.MeshesData.size(); i++)
			{
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
		
					CommandList->SetGraphicRootConstant(Renderer->ViewBuffer->GetViewIndex(), 0);
					CommandList->SetGraphicRootConstant(PrimitiveBuffer->GetViewIndex(), 1);
					CommandList->SetGraphicRootConstant(Renderer::Get()->StaticSamplersBuffer->GetViewIndex(), 2);
		
					RenderProxy.m_StaticMeshVertexBuffer->Bind(CommandList);
					BindInstanceBuffers(CommandList);

					uint32 VertexCount = RenderProxy.VertexData.GetVertexCount();
					uint32 PrimitiveCount = RenderProxy.VertexData.GetPrimitiveCount();
					CommandList->DrawIndexedPrimitive(RenderProxy.m_IndexBuffer, 0, 0, VertexCount, 0, PrimitiveCount, NumInstances);
				}
			}
		}
	}

	void InstancedStaticMeshSceneProxy::RenderPrePass( class D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		const int32 NumInstances = m_OwningInstancedStaticMeshComponent->GetInstanceCount();
		if (NumInstances > 0 && m_Mesh.IsValid())
		{
			const std::string MeshName = Path::GetCleanName(m_Mesh.GetPath());
			SCOPE_STAT_DYNAMIC(MeshName.c_str());
		
			for (size_t i = 0; i < m_Mesh->Data.MeshesData.size(); i++)
			{
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
		
					CommandList->SetGraphicRootConstant(Renderer->ViewBuffer->GetViewIndex(), 0);
					CommandList->SetGraphicRootConstant(PrimitiveBuffer->GetViewIndex(), 1);
					CommandList->SetGraphicRootConstant(Renderer::Get()->StaticSamplersBuffer->GetViewIndex(), 2);
		
					RenderProxy.m_StaticMeshVertexBuffer->Bind(CommandList);
					BindInstanceBuffers(CommandList);

					uint32 VertexCount = RenderProxy.VertexData.GetVertexCount();
					uint32 PrimitiveCount = RenderProxy.VertexData.GetPrimitiveCount();
					CommandList->DrawIndexedPrimitive(RenderProxy.m_IndexBuffer, 0, 0, VertexCount, 0, PrimitiveCount, NumInstances);
				}
			}
		}
	}

	void InstancedStaticMeshSceneProxy::RenderShadowPass( class D3D12CommandList* CommandList, SceneRenderer* Renderer, LightSceneProxy* LightProxy )
	{
		auto GetMaterialShaderForLightType = [](const MaterialShaders& Shaders, ELightType Type)
		{
			switch ( Type )
			{
			case ELightType::PointLight: return Shaders.GetShader(VertexFactoryType::InstancedStaticMesh, EMaterialStage::PointLightShadow);
			case ELightType::SpotLight:
			case ELightType::DirectionalLight: return Shaders.GetShader(VertexFactoryType::InstancedStaticMesh, EMaterialStage::SpotLightShadow);
			case ELightType::SkyLight:
			default: drn_check(false); return Shaders.GetShader(VertexFactoryType::InstancedStaticMesh, EMaterialStage::PointLightShadow);
			}
		};

		const int32 NumInstances = m_OwningInstancedStaticMeshComponent->GetInstanceCount();
		if (NumInstances > 0 && m_Mesh.IsValid())
		{
			const std::string MeshName = Path::GetCleanName(m_Mesh.GetPath());
			SCOPE_STAT_DYNAMIC(MeshName.c_str());
		
			for (size_t i = 0; i < m_Mesh->Data.MeshesData.size(); i++)
			{
				const StaticMeshSlotData& RenderProxy = m_Mesh->Data.MeshesData[i];
				MaterialSlot& Mat = m_Materials[RenderProxy.MaterialIndex];
		
				MaterialShader* MatShader = Mat.GetParentMaterial()->GetShaderParameters().bIsUsedWithInstancedStaticMesh && Mat.GetParentMaterial()->GetShaderParameters().bHasShadowPass
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
		
					RenderProxy.m_StaticMeshVertexBuffer->Bind(CommandList);
					BindInstanceBuffers(CommandList);

					uint32 VertexCount = RenderProxy.VertexData.GetVertexCount();
					uint32 PrimitiveCount = RenderProxy.VertexData.GetPrimitiveCount();
					CommandList->DrawIndexedPrimitive(RenderProxy.m_IndexBuffer, 0, 0, VertexCount, 0, PrimitiveCount, NumInstances);
				}
			}
		}
	}

	void InstancedStaticMeshSceneProxy::RenderDecalPass( class D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{}

#if WITH_EDITOR
	void InstancedStaticMeshSceneProxy::RenderHitProxyPass( class D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		SCOPE_STAT("HitProxyMesh");
		
		const int32 NumInstances = m_OwningInstancedStaticMeshComponent->GetInstanceCount();
		if (NumInstances == 0 || !m_Mesh.IsValid() || !m_Selectable)
		{
			return;
		}
		
		for (size_t i = 0; i < m_Mesh->Data.MeshesData.size(); i++)
		{
			const StaticMeshSlotData& RenderProxy = m_Mesh->Data.MeshesData[i];
			MaterialSlot& Mat = m_Materials[RenderProxy.MaterialIndex];

			MaterialShader* MatShader = Mat.GetParentMaterial()->GetShaderParameters().bIsUsedWithInstancedStaticMesh && Mat.GetParentMaterial()->GetShaderParameters().bHasHitProxyPass
				? Mat.GetParentMaterial()->GetShaders().GetShader(VertexFactoryType::InstancedStaticMesh, EMaterialStage::Hitproxy)
				: nullptr;

			if (MatShader)
			{
				SCOPE_STAT_DYNAMIC(Mat.GetMaterialName().c_str());

				MatShader->Bind(CommandList);
				Mat.GetMaterialInterface()->BindResources(CommandList);
				
				CommandList->SetGraphicRootConstant(Renderer->ViewBuffer->GetViewIndex(), 0);
				CommandList->SetGraphicRootConstant(PrimitiveBuffer->GetViewIndex(), 1);
				CommandList->SetGraphicRootConstant(Renderer::Get()->StaticSamplersBuffer->GetViewIndex(), 2);
		
				RenderProxy.m_StaticMeshVertexBuffer->Bind(CommandList);
				BindInstanceBuffers(CommandList);

				uint32 VertexCount = RenderProxy.VertexData.GetVertexCount();
				uint32 PrimitiveCount = RenderProxy.VertexData.GetPrimitiveCount();
				CommandList->DrawIndexedPrimitive(RenderProxy.m_IndexBuffer, 0, 0, VertexCount, 0, PrimitiveCount, NumInstances);
			}
		}
	}

	void InstancedStaticMeshSceneProxy::RenderSelectionPass( class D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		const int32 NumInstances = m_OwningInstancedStaticMeshComponent->GetInstanceCount();
		if (NumInstances == 0 || !m_Mesh.IsValid() || !m_SelectedInEditor)
			return;
		
		const std::string MeshName = Path::GetCleanName(m_Mesh.GetPath());
		SCOPE_STAT_DYNAMIC(MeshName.c_str());
		
		for (size_t i = 0; i < m_Mesh->Data.MeshesData.size(); i++)
		{
			const StaticMeshSlotData& RenderProxy = m_Mesh->Data.MeshesData[i];
			MaterialSlot& Mat = m_Materials[RenderProxy.MaterialIndex];
		
			MaterialShader* MatShader = Mat.GetParentMaterial()->GetShaderParameters().bIsUsedWithInstancedStaticMesh && Mat.GetParentMaterial()->GetShaderParameters().bHasEditorSelectionPass
				? Mat.GetParentMaterial()->GetShaders().GetShader(VertexFactoryType::InstancedStaticMesh, EMaterialStage::EditorSelection)
				: nullptr;

			if (MatShader)
			{
				SCOPE_STAT_DYNAMIC(Mat.GetMaterialName().c_str());

				MatShader->Bind(CommandList);
				Mat.GetMaterialInterface()->BindResources(CommandList);

				CommandList->SetGraphicRootConstant(Renderer->ViewBuffer->GetViewIndex(), 0);
				CommandList->SetGraphicRootConstant(PrimitiveBuffer->GetViewIndex(), 1);
				CommandList->SetGraphicRootConstant(Renderer::Get()->StaticSamplersBuffer->GetViewIndex(), 2);

				RenderProxy.m_StaticMeshVertexBuffer->Bind(CommandList);
				BindInstanceBuffers(CommandList);

				uint32 VertexCount = RenderProxy.VertexData.GetVertexCount();
				uint32 PrimitiveCount = RenderProxy.VertexData.GetPrimitiveCount();
				CommandList->DrawIndexedPrimitive(RenderProxy.m_IndexBuffer, 0, 0, VertexCount, 0, PrimitiveCount, NumInstances);
			}
		}
	}

	void InstancedStaticMeshSceneProxy::RenderEditorPrimitivePass( class D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		const int32 NumInstances = m_OwningInstancedStaticMeshComponent->GetInstanceCount();
		if (NumInstances == 0 || !m_Mesh.IsValid() || !m_EditorPrimitive)
		{
			return;
		}
		
		const std::string MeshName = Path::GetCleanName(m_Mesh.GetPath());
		SCOPE_STAT_DYNAMIC(MeshName.c_str());
		
		for (size_t i = 0; i < m_Mesh->Data.MeshesData.size(); i++)
		{
			const StaticMeshSlotData& RenderProxy = m_Mesh->Data.MeshesData[i];
			MaterialSlot& Mat = m_Materials[RenderProxy.MaterialIndex];
		
			MaterialShader* MatShader = Mat.GetParentMaterial()->GetShaderParameters().bIsUsedWithInstancedStaticMesh && Mat.GetParentMaterial()->GetShaderParameters().bHasEditorPrimitivePass
				? Mat.GetParentMaterial()->GetShaders().GetShader(VertexFactoryType::InstancedStaticMesh, EMaterialStage::EditorPrimitive)
				: nullptr;

			if (MatShader)
			{
				SCOPE_STAT_DYNAMIC(Mat.GetMaterialName().c_str());
		
				MatShader->Bind(CommandList);
				Mat.GetMaterialInterface()->BindResources(CommandList);

				CommandList->SetGraphicRootConstant(Renderer->ViewBuffer->GetViewIndex(), 0);
				CommandList->SetGraphicRootConstant(PrimitiveBuffer->GetViewIndex(), 1);
				CommandList->SetGraphicRootConstant(Renderer::Get()->StaticSamplersBuffer->GetViewIndex(), 2);
		
				RenderProxy.m_StaticMeshVertexBuffer->Bind(CommandList);
				BindInstanceBuffers(CommandList);

				uint32 VertexCount = RenderProxy.VertexData.GetVertexCount();
				uint32 PrimitiveCount = RenderProxy.VertexData.GetPrimitiveCount();
				CommandList->DrawIndexedPrimitive(RenderProxy.m_IndexBuffer, 0, 0, VertexCount, 0, PrimitiveCount, NumInstances);
			}
		}
	}
#endif

	void InstancedStaticMeshSceneProxy::InitResources( class D3D12CommandList* CommandList )
	{
		
	}

	void InstancedStaticMeshSceneProxy::UpdateResources( class D3D12CommandList* CommandList )
	{
		// TODO: issue when proxy not begin rendered
		//m_PrimitiveData.m_PrevLocalToWorld = m_PrimitiveData.m_LocalToWorld;
		//m_PrimitiveData.m_PrevLocalToProjection = m_PrimitiveData.m_LocalToProjection;

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

			UpdateBuffers(CommandList);
		}

		for (MaterialSlot& MatSlot : m_Materials)
		{
			MatSlot.GetMaterialInterface()->UploadResources(CommandList);
		}

		m_OwningInstancedStaticMeshComponent->ClearRenderStateDirty();
	}

	void InstancedStaticMeshSceneProxy::UpdateBuffers( D3D12CommandList* CommandList )
	{
		//m_PrimitiveData.m_LocalToWorld = Matrix(m_OwningInstancedStaticMeshComponent->GetWorldTransform()).Get();
		//m_PrimitiveData.m_LocalToProjection = XMMatrixMultiply( m_PrimitiveData.m_LocalToWorld.Get(), Renderer->GetSceneView().WorldToProjection.Get() );
		m_PrimitiveData.m_Guid = m_Guid;

		PrimitiveBuffer = RenderUniformBuffer::Create(CommandList->GetParentDevice(), sizeof(PrimitiveData), EUniformBufferUsage::MultiFrame, &m_PrimitiveData);

		const int32 NumInstances = m_OwningInstancedStaticMeshComponent->GetInstanceCount();
		if (NumInstances > 0)
		{
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

			RandomStream RandStream(m_OwningInstancedStaticMeshComponent->GetInstanceRandomSeed());

			OriginRandom.resize(NumInstances);
			LocalToWorldMatrices.resize(NumInstances);
			for (int32 InstanceIndex = 0; InstanceIndex < NumInstances; InstanceIndex++)
			{
				Transform LocalToWorldTransform;
				m_OwningInstancedStaticMeshComponent->GetInstanceTransform(InstanceIndex, LocalToWorldTransform, true);

				LocalToWorldMatrices[InstanceIndex] = Matrix16_4x3(LocalToWorldTransform);
				OriginRandom[InstanceIndex] = Vector4(LocalToWorldTransform.GetLocation(), RandStream.GetFraction());
			}

			CreateVertexBufferConditional(true, CommandList, OriginRandomBuffer, (void*)OriginRandom.data(), NumInstances * sizeof(Vector4), "VB_PositionRandom");
			CreateVertexBufferConditional(true, CommandList, LocalToWorldBuffer, (void*)LocalToWorldMatrices.data(), NumInstances * sizeof(Matrix16_4x3), "VB_LocalToWorld");

			for (int32 CustomDataIndex = 0; CustomDataIndex < NUM_INSTANCED_CUSTOM_DATA; CustomDataIndex++)
			{
				CreateVertexBufferConditional(m_OwningInstancedStaticMeshComponent->bCustomData[CustomDataIndex], CommandList, CustomDataBuffers[CustomDataIndex],
					(void*)m_OwningInstancedStaticMeshComponent->CustomData[CustomDataIndex].data(), NumInstances * sizeof(Vector4), "VB_CustomData" + std::to_string(CustomDataIndex));
			}
		}
		else
		{
			OriginRandomBuffer = nullptr;
			LocalToWorldBuffer = nullptr;
			for (int32 CustomDataIndex = 0; CustomDataIndex < NUM_INSTANCED_CUSTOM_DATA; CustomDataIndex++)
			{
				CustomDataBuffers[CustomDataIndex] = nullptr;
			}
		}
	}

	void InstancedStaticMeshSceneProxy::BindInstanceBuffers( D3D12CommandList* CommandList )
	{
		CommandList->SetStreamSource(8, OriginRandomBuffer, 0);
		CommandList->SetStreamSource(9, LocalToWorldBuffer, 0);
		for (int32 CustomDataIndex = 0; CustomDataIndex < NUM_INSTANCED_CUSTOM_DATA; CustomDataIndex++)
		{
			CommandList->SetStreamSource(10 + CustomDataIndex, CustomDataBuffers[CustomDataIndex], 0);
		}
	}

        }  // namespace Drn