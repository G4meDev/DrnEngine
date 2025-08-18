#pragma once

#include "ForwardTypes.h"
#include "LightComponent.h"

namespace Drn
{
	class SkyLightComponent : public LightComponent
	{
	public:
		SkyLightComponent();
		virtual ~SkyLightComponent();
	
		virtual void Serialize( Archive& Ar ) override;

		inline const AssetHandle<TextureCube> GetCubemap() const { return m_Cubemap.m_TextureCube; }
		inline bool GetBlockLowerHemisphere() const { return m_BlockLowerHemesphere; }
		inline const Vector GetLowerHemisphereColor() const { return m_LowerHemesphereColor; }

		void SetCubemap(const AssetHandle<TextureCube>& Cubemap);
		void SetBlockLowerHemisphere(bool Block);
		void SetLowerHemisphereColor(const Vector& InColor);

	protected:
		virtual void RegisterComponent( World* InOwningWorld ) override;
		virtual void UnRegisterComponent() override;
	
		virtual void OnUpdateTransform( bool SkipPhysic ) override;

		class SkyLightSceneProxy* m_SkyLightSceneProxy;

		TextureCubeProperty m_Cubemap;

		bool m_BlockLowerHemesphere;
		Vector m_LowerHemesphereColor;

#if WITH_EDITOR
		inline virtual bool HasSprite() const override
		{
			return true;
		}
	
		virtual void DrawDetailPanel( float DeltaTime ) override;
#endif
	
	private:
	};
}