#pragma once

#include "ForwardTypes.h"
#include "Runtime/Engine/EngineTypes.h"

namespace Drn
{
	class LightSceneProxy
	{
	public:
		virtual ~LightSceneProxy();

		inline bool IsMarkedPendingKill() const { return bPendingDestory; }
		inline void MarkPendingKill() { bPendingDestory = true; }

		inline void SetColor ( const Vector& Color ) { m_LightColor = Color; }
		inline void SetCastShadow( bool CastShadow ) { m_CastShadow = CastShadow; }
		inline void SetCastStaticShadow( bool CastShadow ) { bCastStaticShadow = CastShadow; }
		inline void SetCastDynamicShadow( bool CastShadow ) { bCastDynamicShadow = CastShadow; }

		virtual void Render( class D3D12CommandList* CommandList, SceneRenderer* Renderer ) = 0;
		virtual void RenderShadowDepth( class D3D12CommandList* CommandList, SceneRenderer* Renderer ) = 0;

		virtual void UpdateResources( class D3D12CommandList* CommandList ) = 0;

		inline const Vector& GetColor() const { return m_LightColor; }

		inline virtual ELightType GetLightType() const = 0;

		virtual float GetMaxDrawDistance() const = 0;
		virtual Sphere GetBoundingSphere() const = 0;

		
		inline virtual void SetStatic(bool bInStatic) { bStatic = bInStatic; }
		inline bool IsStatic() const { return bStatic; }

	protected:
		LightSceneProxy( class LightComponent* InComponent );

		class LightComponent* m_LightComponent;

		Vector m_LightColor;

		bool m_CastShadow : 1;
		bool bCastStaticShadow : 1;
		bool bCastDynamicShadow : 1;

#if D3D12_Debug_INFO
		std::string m_Name;
#endif

		bool bStatic;
		bool bPendingDestory;

		friend class LightComponent;

	private:

	};
}