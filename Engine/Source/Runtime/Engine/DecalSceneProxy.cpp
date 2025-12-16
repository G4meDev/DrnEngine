#include "DrnPCH.h"
#include "DecalSceneProxy.h"
#include "Runtime/Components/DecalComponent.h"

namespace Drn
{
	DecalSceneProxy::DecalSceneProxy( class DecalComponent* InComponent )
		: m_DecalComponent( InComponent )
	{}

	DecalSceneProxy::~DecalSceneProxy()
	{
		ReleaseBuffers();
	}

	void DecalSceneProxy::UpdateResources( D3D12CommandList* CommandList )
	{
		if (m_DecalComponent)
		{
			m_DecalComponent->UpdateRenderStateConditional();
		}

		if (m_Material.IsValid())
		{
			m_Material->UploadResources(CommandList);
		}
	}

	void DecalSceneProxy::Render( D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		if (!m_Material.IsValid() || m_Material->GetMaterialDomain() != EMaterialDomain::Decal)
		{
			return;
		}

		Matrix LocalToWorldMatrix = Matrix( m_WorldTransform );
		m_DecalData.LocalToProjection = LocalToWorldMatrix * Matrix(Renderer->GetSceneView().WorldToProjection);
		m_DecalData.ProjectionToLocal = Matrix(Renderer->GetSceneView().ProjectionToWorld) * LocalToWorldMatrix.Inverse();

		TRefCountPtr<RenderUniformBuffer> DecalBuffer = RenderUniformBuffer::Create(CommandList->GetParentDevice(), sizeof(DecalData), EUniformBufferUsage::SingleFrame, &m_DecalData);

		CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, DecalBuffer->GetViewIndex(), 1);

		m_Material->BindDeferredDecalPass(CommandList->GetD3D12CommandList());

		CommonResources::Get()->m_UniformCubePositionOnly->BindAndDraw(CommandList);
	}

	void DecalSceneProxy::ReleaseBuffers()
	{}
}