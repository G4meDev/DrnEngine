#pragma once

#include "ForwardTypes.h"
#include "LightSceneProxy.h"

namespace Drn
{
	struct SkyLightData
	{
	public:
		SkyLightData() = default;
		
		Vector Color;
		uint32 CubemapTexture;

		Vector LowerHemesphereColor;
		uint32 BlockLowerHemesphere;
	};

	class SkyLightSceneProxy : public LightSceneProxy
	{
	public:
		SkyLightSceneProxy( class SkyLightComponent* InComponent );
		virtual ~SkyLightSceneProxy();

		inline virtual ELightType GetLightType() const { return ELightType::SkyLight; };

		const AssetHandle<TextureCube>& GetCubemap() const { return m_Cubemap; }

		virtual void Render( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer ) override;
		virtual void RenderShadowDepth( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer ) override;

		void UpdateResources( ID3D12GraphicsCommandList2* CommandList ) override;

	protected:

		SkyLightComponent* m_SkyLightComponent = nullptr;

		AssetHandle<TextureCube> m_Cubemap;
		bool m_BlockLowerHemesphere;
		Vector m_LowerHemesphereColor;

		SkyLightData m_SkyLightData;
		Resource* m_LightBuffer;

	private:

	};
}
