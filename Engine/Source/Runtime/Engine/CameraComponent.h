#pragma once

#include "ForwardTypes.h"

using namespace DirectX;

namespace Drn
{
	class CameraComponent
	{
	public:

		CameraComponent();
		virtual ~CameraComponent();

		XMVECTOR m_Pos;
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
