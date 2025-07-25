#pragma once

namespace Drn
{
	class LightSceneProxy
	{
	public:

		inline void Release() { delete this; }

		inline void SetLocalToWorld( const Matrix& InLocalToWorld ) { m_LocalToWorld = InLocalToWorld.Get(); }
		inline void SetWorldPosition( const Vector& InWorldPosition ) { m_WorldPosition = InWorldPosition; }
		inline void SetColor ( const Vector& Color ) { m_LightColor = Color; }
		inline void SetCastShadow( bool CastShadow ) { m_CastShadow = CastShadow; }

		virtual void Render( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer ) = 0;
		virtual void RenderShadowDepth( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer ) = 0;

#if WITH_EDITOR
		virtual void DrawAttenuation(World* InWorld) = 0;
		inline void SetSelectedInEditor( bool Selected ) { m_SelectedInEditor = Selected; }
		bool m_SelectedInEditor = false;
#endif

	protected:
		LightSceneProxy( class LightComponent* InComponent );
		virtual ~LightSceneProxy();

		class LightComponent* m_LightComponent;

		XMMATRIX m_LocalToWorld;

		Vector m_WorldPosition;
		Vector m_LightColor;
		bool m_CastShadow;

	private:

	};
}