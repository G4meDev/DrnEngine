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

		void UpdateResources( ID3D12GraphicsCommandList2* CommandList ) override;

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

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DsvHeap;
		
		Resource* m_ShadowmapResource;
		std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> m_ShadowmapCpuHandles;

		DirectionalLightData m_LightData;
		Resource* m_LightBuffer[NUM_BACKBUFFERS] = {nullptr};

		DirectionalLightShadowData m_ShadowData;
		Resource* m_ShadowBuffer[NUM_BACKBUFFERS] = {nullptr};

		std::vector<Resource*> m_CsWorldToProjectionMatricesBuffer[NUM_BACKBUFFERS];

		friend class DirectionalLightComponent;

	private:
		
	};
}