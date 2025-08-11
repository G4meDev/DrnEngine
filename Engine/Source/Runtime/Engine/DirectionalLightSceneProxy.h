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

		Matrix WorldToProjectionMatrices;

		uint32 ShadowmapTextureIndex;
		float DepthBias;
		float InvShadowResolution;
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

		float m_CSLogDistribution = 0.8f;
		float m_CsZScale = 1.0f;
		std::vector<float> m_SplitDistances;
		std::vector<Matrix> m_CSWorldToProjetcionMatrices;

		Sphere GetShadowSplitBounds( const SceneRendererView& View, int32 CascadeIndex );
		void CalculateSplitDistance();
		Sphere GetShadowSplitBoundsDepthRange( const SceneRendererView& View, const Vector& ViewOrigin, float SplitNear, float SplitFar );

#if WITH_EDITOR
		virtual void DrawAttenuation(World* InWorld) override;
#endif

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DsvHeap;
		
		Resource* m_ShadowmapResource;
		D3D12_CPU_DESCRIPTOR_HANDLE m_ShadowmapCpuHandle;

		DirectionalLightData m_LightData;
		Resource* m_LightBuffer;

		DirectionalLightShadowData m_ShadowData;
		Resource* m_ShadowBuffer;

	};
}