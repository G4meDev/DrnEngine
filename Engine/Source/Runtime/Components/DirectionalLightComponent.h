#pragma once

#include "ForwardTypes.h"
#include "Runtime/Components/LightComponent.h"

namespace Drn
{
	class DirectionalLightComponent : public LightComponent
	{
	public:
		DirectionalLightComponent();
		virtual ~DirectionalLightComponent();

		virtual void Serialize( Archive& Ar ) override;

		void SetCascadeCount(int32 CascadeCount);
		void SetShadowDistance(float ShadowDistance);
		void SetCascadeLogDistribution(float LogDistribution);
		void SetCascadeDepthScale(float DepthScale);

		inline float GetShadowDistance() const			{ return m_ShadowDistance; }
		inline int32 GetCascadeCount() const			{ return m_CascadeCount; }
		inline float GetCascadeLogDistribution() const	{ return m_CascadeLogDistribution; }
		inline float GetCascadeDepthScale() const		{ return m_CascadeDepthScale; }

#if WITH_EDITOR
		virtual void DrawDetailPanel( float DeltaTime ) override;

		virtual void DrawEditorSelected() override;
#endif
	protected:

		virtual void RegisterComponent(World* InOwningWorld) override;
		virtual void UnRegisterComponent() override;

		virtual void OnUpdateTransform( bool SkipPhysic ) override;

		int32 m_CascadeCount;
		float m_ShadowDistance;
		float m_CascadeLogDistribution;
		float m_CascadeDepthScale;

		class DirectionalLightSceneProxy* m_DirectionalLightSceneProxy;

#if WITH_EDITOR
		inline virtual bool HasSprite() const override { return true; }
#endif
	};
}