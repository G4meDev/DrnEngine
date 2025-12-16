#pragma once

#include "ForwardTypes.h"
#include "Runtime/Engine/LightSceneProxy.h"
#include "Runtime/Renderer/SceneRenderer.h"

namespace Drn
{
	struct DirectionalLightData
	{
		DirectionalLightData() = default;

		Vector Direction;
		uint32 ShadowmapBufferIndex;

		Vector Color;
		float Padding;
	};

	struct DirectionalLightShadowData
	{
		DirectionalLightShadowData() = default;

		float DepthBias;
		float InvShadowResolution;
		uint32 CacadeCount;
		uint32 ShadowmapTextureIndex;

		Matrix CsWorldToProjectionMatrices[8];
		float SplitDistances[8];
	};

	class DirectionalLightSceneProxy : public LightSceneProxy
	{
	public:
		DirectionalLightSceneProxy( class DirectionalLightComponent* InComponent );
		virtual ~DirectionalLightSceneProxy();

		inline virtual ELightType GetLightType() const { return ELightType::DirectionalLight; };

		virtual void Render( class D3D12CommandList* CommandList, SceneRenderer* Renderer ) override;
		virtual void RenderShadowDepth( class D3D12CommandList* CommandList, SceneRenderer* Renderer ) override;

		void AllocateShadowmap( class D3D12CommandList* CommandList );
		void ReleaseShadowmap();

		void ReleaseBuffers();

		void UpdateResources( class D3D12CommandList* CommandList ) override;

	protected:

		DirectionalLightComponent* m_DirectionalLightComponent = nullptr;

		std::vector<float> m_SplitDistances;

		Vector m_Direction;
		float m_ShadowDistance;
		int32 m_CascadeCount;
		float m_CascadeLogDistribution;
		float m_CascadeDepthScale;
		float m_DepthBias;

		void CalculateSplitDistance();
		Matrix GetShadowSplitBoundsMatrix( const SceneRendererView& View, const Vector& ViewOrigin, float SplitNear, float SplitFar );

		TRefCountPtr<class RenderTexture2DArray> m_ShadowmapResource;
		std::vector<TRefCountPtr<class DepthStencilView>> m_ShadowmapViews;

		DirectionalLightData m_LightData;

		DirectionalLightShadowData m_ShadowData;
		Resource* m_ShadowBuffer[NUM_BACKBUFFERS] = {nullptr};

		std::vector<Resource*> m_CsWorldToProjectionMatricesBuffer[NUM_BACKBUFFERS];

		friend class DirectionalLightComponent;

	private:
		
	};
}