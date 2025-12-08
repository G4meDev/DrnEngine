#pragma once

#include "ForwardTypes.h"
#include "Runtime/Engine/EngineTypes.h"

namespace Drn
{
	class LightSceneProxy
	{
	public:

		inline void Release() { delete this; }

		inline void SetColor ( const Vector& Color ) { m_LightColor = Color; }
		inline void SetCastShadow( bool CastShadow ) { m_CastShadow = CastShadow; }

		virtual void Render( class D3D12CommandList* CommandList, SceneRenderer* Renderer ) = 0;
		virtual void RenderShadowDepth( class D3D12CommandList* CommandList, SceneRenderer* Renderer ) = 0;

		virtual void UpdateResources( class D3D12CommandList* CommandList ) = 0;

		inline const Vector& GetColor() const { return m_LightColor; }

		inline virtual ELightType GetLightType() const = 0;

	protected:
		LightSceneProxy( class LightComponent* InComponent );
		virtual ~LightSceneProxy();

		class LightComponent* m_LightComponent;

		Vector m_LightColor;
		bool m_CastShadow;

#if D3D12_Debug_INFO
		std::string m_Name;
#endif

		friend class LightComponent;

	private:

	};
}