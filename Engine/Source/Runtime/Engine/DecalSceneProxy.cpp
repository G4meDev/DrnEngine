#include "DrnPCH.h"
#include "DecalSceneProxy.h"
#include "Runtime/Components/DecalComponent.h"

namespace Drn
{
	DecalSceneProxy::DecalSceneProxy( class DecalComponent* InComponent )
		: m_DecalComponent( InComponent )
		, bPendingDestory(false)
	{}

	DecalSceneProxy::~DecalSceneProxy()
	{
	}

	void DecalSceneProxy::UpdateResources( D3D12CommandList* CommandList )
	{
		if (m_DecalComponent)
		{
			m_DecalComponent->UpdateRenderStateConditional();
		}

		if (m_Material.IsValid())
		{
			m_Material.GetMaterialInterface()->UploadResources(CommandList);
		}
	}

	void DecalSceneProxy::Render( D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		if (!m_Material.IsValid() || m_Material.GetParentMaterial()->GetMaterialDomain() != EMaterialDomain::Decal)
		{
			return;
		}

		MaterialShader* MatShader = m_Material.GetParentMaterial()->GetShaderParameters().bIsUsedWithDecal && m_Material.GetParentMaterial()->GetShaderParameters().bHasDecalPass
			? m_Material.GetParentMaterial()->GetShaders().GetShader(VertexFactoryType::Decal, EMaterialStage::Decal)
			: nullptr;

		if (MatShader)
		{
			SCOPE_STAT_DYNAMIC(m_Material.GetMaterialName().c_str());

			Matrix LocalToWorldMatrix = Matrix( m_WorldTransform );
			m_DecalData.LocalToProjection = LocalToWorldMatrix * Matrix(Renderer->GetSceneView().WorldToProjection);
			m_DecalData.ProjectionToLocal = Matrix(Renderer->GetSceneView().ProjectionToWorld) * LocalToWorldMatrix.Inverse();

			TRefCountPtr<RenderUniformBuffer> DecalBuffer = RenderUniformBuffer::Create(CommandList->GetParentDevice(), sizeof(DecalData), EUniformBufferUsage::SingleFrame, &m_DecalData);

			CommandList->SetGraphicRootConstant(DecalBuffer->GetViewIndex(), 1);

			MatShader->Bind(CommandList);
			m_Material.GetMaterialInterface()->BindResources(CommandList);

			CommonResources::Get()->m_UniformCubePositionOnly->BindAndDraw(CommandList);
		}
	}

	BoxSphereBounds DecalSceneProxy::GetBounds()
	{
		// TODO: only update bounds when dirty
		if (m_DecalComponent)
		{
			Bounds = m_DecalComponent->GetBounds();
		}

		return Bounds;
	}

}