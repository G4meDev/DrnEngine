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
		uint32 ShadowmapIndex;
	};

	struct ShadowDepthData
	{
	public:
		ShadowDepthData() = default;

		Matrix WorldToProjectionMatrices[6];
		Vector LightPosition;
		float NearZ;
		float Radius;
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

		void CalculateLocalToProjectionForDirection(Matrix& Mat, const Vector& Direction, const Vector& UpVector);

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DsvHeap;

		Resource* m_ShadowCubemapResource;
		D3D12_CPU_DESCRIPTOR_HANDLE m_ShadowmapCpuHandle;

		PointLightBuffer m_Buffer;
		Resource* m_LightBuffer;

		ShadowDepthData m_ShadowDepthData;
		Resource* m_ShadowDepthBuffer;

		D3D12_VIEWPORT m_ShadowViewport;

	private:

	};
}