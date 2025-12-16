#include "DrnPCH.h"
#include "SkyLightSceneProxy.h"
#include "Runtime/Components/SkyLightComponent.h"

namespace Drn
{
	SkyLightSceneProxy::SkyLightSceneProxy( class SkyLightComponent* InComponent )
		: LightSceneProxy( InComponent )
		, m_SkyLightComponent(InComponent)
		, m_Cubemap("")
		, m_BlockLowerHemesphere(false)
		, m_LowerHemesphereColor()
	{}

	SkyLightSceneProxy::~SkyLightSceneProxy()
	{}

	void SkyLightSceneProxy::Render( D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		m_SkyLightData.Color = m_LightColor;
		m_SkyLightData.BlockLowerHemesphere = m_BlockLowerHemesphere;
		m_SkyLightData.LowerHemesphereColor = m_LowerHemesphereColor;
		m_SkyLightData.CubemapTexture = m_Cubemap.IsValid() ? m_Cubemap->GetTextureIndex() : 0;

		TRefCountPtr<RenderUniformBuffer> LightBuffer = RenderUniformBuffer::Create(CommandList->GetParentDevice(), sizeof(SkyLightData), EUniformBufferUsage::SingleFrame, &m_SkyLightData);

		CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, LightBuffer->GetViewIndex(), 1);

		// TODO: make light flags enum. e. g. 1: Pointlight. 2: Spotlight. 3: RectLight. 4: Dynamic. ...
		uint32 LightFlags = 8;
		CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, LightFlags, 7);

		CommonResources::Get()->m_BackfaceScreenTriangle->BindAndDraw(CommandList);
	}

	void SkyLightSceneProxy::RenderShadowDepth( D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{}

	void SkyLightSceneProxy::UpdateResources( D3D12CommandList* CommandList )
	{
		if (m_SkyLightComponent && m_SkyLightComponent->IsRenderStateDirty())
		{
			m_SkyLightComponent->ClearRenderStateDirty();

			m_LightColor = m_SkyLightComponent->GetScaledColor();
			m_Cubemap = m_SkyLightComponent->GetCubemap();
			m_BlockLowerHemesphere = m_SkyLightComponent->GetBlockLowerHemisphere();
			m_LowerHemesphereColor = m_SkyLightComponent->GetLowerHemisphereColor();
		}

		if (m_Cubemap.IsValid())
		{
			m_Cubemap->UploadResources(CommandList);
		}
	}

}