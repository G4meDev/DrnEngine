#pragma once

namespace Drn
{
	class LightSceneProxy
	{
	public:

		inline void Release() { delete this; }

		inline void SetWorldPosition( const Vector& InWorldPosition ) { m_WorldPosition = InWorldPosition; }

#if WITH_EDITOR
		virtual void DrawAttenuation(World* InWorld) = 0;
		bool m_SelectedInEditor = false;
#endif

	protected:
		LightSceneProxy( class LightComponent* InComponent );
		virtual ~LightSceneProxy();

		class LightComponent* m_LightComponent;

		Vector m_WorldPosition;
		Vector m_LightColor;


	private:

	};
}