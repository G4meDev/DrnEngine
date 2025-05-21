#pragma once

namespace Drn
{
	class LightSceneProxy
	{
	public:

	protected:
		LightSceneProxy( class LightComponent* InComponent );
		virtual ~LightSceneProxy();

		class LightComponent* m_LightComponent;

		Vector m_WorldPosition;
		Vector m_LightColor;


	private:

	};
}