#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class ReflectionCaptureComponent;

	class ReflectionCaptureProxy
	{
	public:
		ReflectionCaptureProxy(ReflectionCaptureComponent* InOwningComponent);

		void UpdateResources( class D3D12CommandList* CommandList );

		uint32 CubemapIndex;
		Vector Position;
		float InfluenceRadius;
		Vector CaptureOffset;
		float Brightness;

		inline bool IsMarkedPendingDestroy() const { return bPendingDestory; };
		inline void MarkPendingDestroy() { bPendingDestory = true; };

	protected:

		bool bPendingDestory;
		ReflectionCaptureComponent* OwningReflectionComponent;
	};
}