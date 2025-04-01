#pragma once

#if WITH_EDITOR

#include "ForwardTypes.h"

namespace Drn
{
	class ViewportPanel
	{
	public:
		ViewportPanel();
		~ViewportPanel();

		void Draw(float DeltaTime);

	protected:

		void OnViewportSizeChanged( const IntPoint& NewSize );

		D3D12_CPU_DESCRIPTOR_HANDLE ViewCpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE ViewGpuHandle;

		IntPoint CachedSize = IntPoint( 1920, 1080 );

		ViewportCameraInputHandler CameraInputHandler;
		float CameraMovementSpeed = 0.01f;
		float CameraRotationSpeed = 0.01f;

	private:
	};
}

#endif