#pragma once

#include "ForwardTypes.h"
#include "LightSceneProxy.h"

namespace Drn
{
	struct PointLightBuffer
	{
	public:
		PointLightBuffer() = default;

		Vector WorldPosition;
		float Scale;

		Vector Color;
		float InvRadius;

		uint32 ShadowBufferIndex;
	};

	struct ShadowDepthData
	{
	public:
		ShadowDepthData() = default;

		Matrix WorldToProjectionMatrices[6];
		uint32 ShadowmapTextureIndex;
		float DepthBias;
		float InvShadowResolution;
	};

	class PointLightSceneProxy : public LightSceneProxy
	{
	public:
		PointLightSceneProxy( class PointLightComponent* InComponent );
		virtual ~PointLightSceneProxy();

		inline virtual ELightType GetLightType() const { return ELightType::PointLight; };

		virtual void Render( class D3D12CommandList* CommandList, SceneRenderer* Renderer ) override;
		virtual void RenderShadowDepth( class D3D12CommandList* CommandList, SceneRenderer* Renderer ) override;

		void AllocateShadowmap(class D3D12CommandList* CommandList);
		void ReleaseShadowmap();

		inline void SetRadius( float Radius ) { m_Radius = Radius; }
		inline void SetDepthBias( float Bias ) { m_DepthBias = Bias; }

		void UpdateResources( class D3D12CommandList* CommandList ) override;
		void ReleaseBuffer();

	protected:

		Vector m_WorldPosition;

		float m_Radius;
		float m_DepthBias;

		class PointLightComponent* m_PointLightComponent;

		void CalculateLocalToProjectionForDirection(Matrix& Mat, const Vector& Direction, const Vector& UpVector);

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DsvHeap;

		Resource* m_ShadowCubemapResource;
		D3D12_CPU_DESCRIPTOR_HANDLE m_ShadowmapCpuHandle;

		PointLightBuffer m_Buffer;
		Resource* m_LightBuffer;

		ShadowDepthData m_ShadowDepthData;
		Resource* m_ShadowDepthBuffer;

	private:

	};
}