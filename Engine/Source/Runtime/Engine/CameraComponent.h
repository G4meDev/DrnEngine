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

		inline virtual EComponentType GetComponentType() override { return EComponentType::CameraComponent; }

		XMVECTOR m_FocusPoint;
		XMVECTOR m_UpVector;

		XMVECTOR m_Rotation;

		float m_AspectRatio;
		float m_FOV;

		float m_ClipMin;
		float m_ClipMax;

		void CalculateMatrices( XMMATRIX& InViewMatrix, XMMATRIX& InProjectionMatrix, float AspectRatio);

	protected:

	private:
	};
}
