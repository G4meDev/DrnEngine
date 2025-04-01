#pragma once

#if WITH_EDITOR

#include "ForwardTypes.h"

namespace Drn
{
	struct ViewportCameraInputHandler
	{
	public:
		ViewportCameraInputHandler();

		bool Tick( float DeltaTime );

		IntPoint m_MouseDelta;
		DirectX::XMVECTOR m_Displacement;
	
	private:

		IntPoint m_LastMousePos;
		bool m_CapturingMouse;
	};
}

#endif