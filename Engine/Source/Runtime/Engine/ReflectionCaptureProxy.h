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

		bool HasValidCubemap() const;
		class RenderTextureCube* GetCubemap() const;

		Vector GetPosition() const;
		float GetRadius() const;
		Vector GetCaptureOffset() const;
		float GetBrightness() const;

	protected:

		bool bPendingDestory;
		ReflectionCaptureComponent* OwningReflectionComponent;
	};
}