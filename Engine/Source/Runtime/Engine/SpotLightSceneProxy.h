#pragma once

#include "ForwardTypes.h"
#include "LightSceneProxy.h"

namespace Drn
{
	struct SpotLightData
	{
	public:
		SpotLightData() = default;

		Vector Direction;
		float Attenuation;
		float OutterRadius;
		float InnerRadius;
	};

	//struct PointLightBuffer
	//{
	//public:
	//	PointLightBuffer() = default;
	//
	//	Matrix LocalToProjection;
	//	Vector CameraPosition;
	//	float Pad_1;
	//	Vector WorldPosition;
	//	float Radius;
	//	Vector LightColor;
	//	uint32 ShadowmapIndex;
	//};

	//struct ShadowDepthData
	//{
	//public:
	//	ShadowDepthData() = default;
	//
	//	Matrix WorldToProjectionMatrices[6];
	//	Vector LightPosition;
	//	float NearZ;
	//	float Radius;
	//	float DepthBias;
	//	float InvShadowResolution;
	//};

	class SpotLightSceneProxy : public LightSceneProxy
	{
	public:
		SpotLightSceneProxy( class SpotLightComponent* InComponent );
		virtual ~SpotLightSceneProxy();

		virtual void Render( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer ) override;
		virtual void RenderShadowDepth( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer ) override;

		//void AllocateShadowmap(ID3D12GraphicsCommandList2* CommandList);
		//void ReleaseShadowmap();
		//
		//inline void SetRadius( float Radius ) { m_Radius = Radius; }
		//inline void SetDepthBias( float Bias ) { m_DepthBias = Bias; }

		inline void SetDirection( const Vector& Direction ) { m_SpotLightData.Direction = Direction; }
		inline void SetAttenuation( float Attenuation ) { m_SpotLightData.Attenuation = Attenuation; }
		inline void SetOutterRadius( float OutterRadius ) { m_SpotLightData.OutterRadius = OutterRadius; }
		inline void SetInnerRadius( float InnerRadius ) { m_SpotLightData.InnerRadius = InnerRadius; }

	protected:
		//float m_Radius;
		//float m_DepthBias;

		SpotLightData m_SpotLightData;

#if WITH_EDITOR
		virtual void DrawAttenuation(World* InWorld) override;
#endif

		//void CalculateLocalToProjectionForDirection(Matrix& Mat, const Vector& Direction, const Vector& UpVector);
		//
		//Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DsvHeap;
		//
		//Resource* m_ShadowCubemapResource;
		//D3D12_CPU_DESCRIPTOR_HANDLE m_ShadowmapCpuHandle;
		//
		//PointLightBuffer m_Buffer;
		//Resource* m_LightBuffer;
		//
		//ShadowDepthData m_ShadowDepthData;
		//Resource* m_ShadowDepthBuffer;
		//
		//D3D12_VIEWPORT m_ShadowViewport;

	private:

	};
}
