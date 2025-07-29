#pragma once

#include "ForwardTypes.h"
#include "LightSceneProxy.h"

namespace Drn
{
	struct PointLightBuffer
	{
	public:
		PointLightBuffer() = default;

		Matrix LocalToProjection;
		Vector CameraPosition;
		float Pad_1;
		Vector WorldPosition;
		float Radius;
		Vector LightColor;
	};

	class PointLightSceneProxy : public LightSceneProxy
	{
	public:
		PointLightSceneProxy( class PointLightComponent* InComponent );
		virtual ~PointLightSceneProxy();

		virtual void Render( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer ) override;
		virtual void RenderShadowDepth( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer ) override;

		void AllocateShadowmap(ID3D12GraphicsCommandList2* CommandList);
		void ReleaseShadowmap();

		inline void SetRadius( float Radius ) { m_Radius = Radius; }

	protected:
		float m_Radius;


#if WITH_EDITOR
		virtual void DrawAttenuation(World* InWorld) override;
#endif

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DsvHeap;

		Resource* m_ShadowCubemapResource;
		D3D12_CPU_DESCRIPTOR_HANDLE m_ShadowmapCpuHandle;


		PointLightBuffer m_Buffer;
		Resource* m_LightBuffer;

		D3D12_CPU_DESCRIPTOR_HANDLE m_LightBufferCpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE m_LightBufferGpuHandle;


	private:

	};
}