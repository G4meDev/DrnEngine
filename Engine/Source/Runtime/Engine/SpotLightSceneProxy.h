#pragma once

#include "ForwardTypes.h"
#include "LightSceneProxy.h"

namespace Drn
{
	struct SpotLightData
	{
	public:
		SpotLightData() = default;

		Matrix LocalToWorld;
		
		Vector WorldPosition;
		float Attenuation;

		Vector Direction;
		float InvRadius;
		
		Vector Color;
		float OutterRadius;
		
		float InnerRadius;
		float CosOuterCone;
		float InvCosConeDifference;
		uint32 ShadowBufferIndex;
	};

	struct SpotLightShadowData
	{
	public:
		SpotLightShadowData() = default;
	
		Matrix WorldToProjectionMatrices;
		uint32 ShadowmapTextureIndex;
		float DepthBias;
		float InvShadowResolution;
	};

	class SpotLightSceneProxy : public LightSceneProxy
	{
	public:
		SpotLightSceneProxy( class SpotLightComponent* InComponent );
		virtual ~SpotLightSceneProxy();

		inline virtual ELightType GetLightType() const { return ELightType::SpotLight; };

		virtual void Render( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer ) override;
		virtual void RenderShadowDepth( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer ) override;

		void AllocateShadowmap(ID3D12GraphicsCommandList2* CommandList);
		void ReleaseShadowmap();

		inline void SetDirection( const Vector& Direction ) { m_SpotLightData.Direction = Direction; }
		inline void SetAttenuation( float Attenuation ) { m_SpotLightData.Attenuation = Attenuation; }
		inline void SetOutterRadius( float OutterRadius ) { m_SpotLightData.OutterRadius = OutterRadius; }
		inline void SetInnerRadius( float InnerRadius ) { m_SpotLightData.InnerRadius = InnerRadius; }

		void UpdateResources( ID3D12GraphicsCommandList2* CommandList ) override;

	protected:

		Matrix m_LocalToWorld;
		Vector m_WorldPosition;
		Vector m_Direction;

		float m_Attenuation;
		float m_InnerRadius;
		float m_OuterRadius;
		float m_DepthBias;

		SpotLightComponent* m_SpotLightComponent = nullptr;

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DsvHeap;
		
		Resource* m_ShadowmapResource;
		D3D12_CPU_DESCRIPTOR_HANDLE m_ShadowmapCpuHandle;

		SpotLightData m_SpotLightData;
		Resource* m_LightBuffer;

		SpotLightShadowData m_ShadowDepthData;
		Resource* m_ShadowDepthBuffer;

	private:

	};
}
