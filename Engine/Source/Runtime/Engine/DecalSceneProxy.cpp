#include "DrnPCH.h"
#include "DecalSceneProxy.h"
#include "Runtime/Components/DecalComponent.h"

namespace Drn
{
	DecalSceneProxy::DecalSceneProxy( class DecalComponent* InComponent )
		: m_DecalComponent( InComponent )
		, m_DecalBuffer(nullptr)
	{
		m_DecalBuffer = Resource::Create(D3D12_HEAP_TYPE_UPLOAD, CD3DX12_RESOURCE_DESC::Buffer( 256 ), D3D12_RESOURCE_STATE_GENERIC_READ, false);
#if D3D12_Debug_INFO
		m_DecalBuffer->SetName("CB_Decal_Unnaemd");
#endif

		D3D12_CONSTANT_BUFFER_VIEW_DESC ResourceViewDesc = {};
		ResourceViewDesc.BufferLocation = m_DecalBuffer->GetD3D12Resource()->GetGPUVirtualAddress();
		ResourceViewDesc.SizeInBytes = 256;

		Renderer::Get()->GetD3D12Device()->CreateConstantBufferView( &ResourceViewDesc, m_DecalBuffer->GetCpuHandle());
	}

	DecalSceneProxy::~DecalSceneProxy()
	{
		ReleaseBuffers();
	}

	void DecalSceneProxy::UpdateResources( ID3D12GraphicsCommandList2* CommandList )
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

	void DecalSceneProxy::Render( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer )
	{
		if (!m_Material.IsValid() || m_Material->GetMaterialDomain() != EMaterialDomain::Decal)
		{
			return;
		}

		Matrix LocalToWorldMatrix = Matrix( m_WorldTransform );
		m_DecalData.LocalToProjection = LocalToWorldMatrix * Matrix(Renderer->GetSceneView().WorldToProjection);
		m_DecalData.ProjectionToLocal = Matrix(Renderer->GetSceneView().ProjectionToWorld) * LocalToWorldMatrix.Inverse();

		UINT8* ConstantBufferStart;
		CD3DX12_RANGE readRange( 0, 0 );
		m_DecalBuffer->GetD3D12Resource()->Map(0, &readRange, reinterpret_cast<void**>( &ConstantBufferStart ) );
		memcpy( ConstantBufferStart, &m_DecalData, sizeof(DecalData));
		m_DecalBuffer->GetD3D12Resource()->Unmap(0, nullptr);

		CommandList->SetGraphicsRoot32BitConstant(0, Renderer::Get()->GetBindlessSrvIndex(m_DecalBuffer->GetGpuHandle()), 1);

		m_Material->BindMainPass(CommandList);

		CommonResources::Get()->m_UniformCubePositionOnly->BindAndDraw(CommandList);
	}

	void DecalSceneProxy::ReleaseBuffers()
	{
		if (m_DecalBuffer)
		{
			delete m_DecalBuffer;
			m_DecalBuffer = nullptr;
		}
	}
}