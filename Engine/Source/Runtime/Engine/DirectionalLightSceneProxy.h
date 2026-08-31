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

	struct CascadeSplitData
	{
		float SplitNear;
		float SplitFar;
		float SplitLength;
		float DepthBias;
	};

	struct DirectionalLightShadowData
	{
		DirectionalLightShadowData() = default;

		float InvShadowResolution;
		uint32 CacadeCount;
		uint32 ShadowmapTextureIndex;
		float unused_1;

		Matrix CsWorldToProjectionMatrices[8];
		CascadeSplitData SplitData[8];
	};

	class DirectionalLightSceneProxy : public LightSceneProxy
	{
	public:
		DirectionalLightSceneProxy( class DirectionalLightComponent* InComponent );
		virtual ~DirectionalLightSceneProxy();

		inline virtual ELightType GetLightType() const { return ELightType::DirectionalLight; };

		inline Vector GetLightDirection() const { return m_Direction; }

		virtual void Render( class D3D12CommandList* CommandList, SceneRenderer* Renderer ) override;
		virtual void RenderShadowDepth( class D3D12CommandList* CommandList, SceneRenderer* Renderer ) override;

#if WITH_EDITOR
		virtual void BakeShadowDepth( class D3D12CommandList* CommandList, SceneRenderer* Renderer ) override {}
		virtual bool RequiresShadowBake() const override { return false; }
#endif

		void AllocateShadowmap( class D3D12CommandList* CommandList );
		void ReleaseShadowmap();

		void ReleaseBuffers();

		void UpdateResources( class D3D12CommandList* CommandList ) override;

		virtual float GetMaxDrawDistance() const override { return FLT_MAX; }
		virtual Sphere GetBoundingSphere() const override { return Sphere(Vector::ZeroVector, FLT_MAX); };

		Matrix GetLightViewMatrix() const;

	protected:

		DirectionalLightComponent* m_DirectionalLightComponent = nullptr;

		std::vector<float> m_SplitDistances;
		std::vector<OrientedBox> m_CascadeBounds;

		Vector m_Direction;
		float m_ShadowDistance;
		int32 m_CascadeCount;
		float m_CascadeLogDistribution;
		float m_CascadeDepthScale;
		float m_DepthBias;

		void CalculateSplitDistance();
		Matrix GetShadowSplitBoundsMatrix( const SceneRendererView& View, const Vector& ViewOrigin, float SplitNear, float SplitFar, OrientedBox& CascadeBound );

		TRefCountPtr<class RenderTexture2DArray> m_ShadowmapResource;
		std::vector<TRefCountPtr<class DepthStencilView>> m_ShadowmapViews;

		TRefCountPtr<class RenderUniformBuffer> ShadowBuffer;
		DirectionalLightData m_LightData;
		DirectionalLightShadowData m_ShadowData;

		friend class DirectionalLightComponent;

	private:
		
	};
}