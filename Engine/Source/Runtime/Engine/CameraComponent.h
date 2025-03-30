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

		float m_AspectRatio;
		float m_FOV;

		void CalculateMatrices( XMMATRIX& InViewMatrix, XMMATRIX& InProjectionMatrix, float AspectRatio);

	protected:

	private:
	};
}
