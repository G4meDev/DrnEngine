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

		virtual void Render( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer ) override;
		virtual void RenderShadowDepth( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer ) override;

		void AllocateShadowmap(ID3D12GraphicsCommandList2* CommandList);
		void ReleaseShadowmap();

		void ReleaseBuffers();

	protected:

		// TODO: remove
		DirectionalLightComponent* m_DirectionalLightComponent = nullptr;

		float m_CSLogDistribution = 0.65f;
		float m_CsZScale = 2.0f;
		std::vector<float> m_SplitDistances;
		std::vector<Matrix> m_CSWorldToProjetcionMatrices;

		void CalculateSplitDistance();
		Matrix GetShadowSplitBoundsMatrix( const SceneRendererView& View, const Vector& ViewOrigin, float SplitNear, float SplitFar );

#if WITH_EDITOR
		virtual void DrawAttenuation(World* InWorld) override;
#endif

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DsvHeap;
		
		Resource* m_ShadowmapResource;
		std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> m_ShadowmapCpuHandles;

		DirectionalLightData m_LightData;
		Resource* m_LightBuffer;

		DirectionalLightShadowData m_ShadowData;
		Resource* m_ShadowBuffer;

		std::vector<Resource*> m_CsWorldToProjectionMatricesBuffer;
	};
}