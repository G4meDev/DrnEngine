#pragma once

#include "ForwardTypes.h"
#include "SceneComponent.h"

using namespace DirectX;

namespace Drn
{
	class CameraComponent : public SceneComponent
	{
	public:

		CameraComponent();
		virtual ~CameraComponent();

		virtual void Tick(float DeltaTime) override;

		virtual void Serialize( Archive& Ar ) override;

		inline virtual EComponentType GetComponentType() override { return EComponentType::CameraComponent; }

		bool m_Perspective;
		float m_FOV;
		float m_OrthoWidth;
		float m_ClipMin;
		float m_ClipMax;

		virtual void GetCameraView(ViewInfo& Info);

#if WITH_EDITOR
		virtual void DrawDetailPanel(float DeltaTime) override;
		void DrawFrustum();

		virtual void DrawEditorDefault() override;
		virtual void DrawEditorSelected() override;
#endif

	protected:

	private:
	};
}
