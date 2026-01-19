#include "DrnPCH.h"
#include "ReflectionCaptureProxy.h"
#include "Runtime/Engine/ReflectionCaptureComponent.h"

namespace Drn
{
	ReflectionCaptureProxy::ReflectionCaptureProxy( ReflectionCaptureComponent* InOwningComponent )
		: OwningReflectionComponent(InOwningComponent)
		, bPendingDestory(false)
	{
		
	}

	void ReflectionCaptureProxy::UpdateResources( class D3D12CommandList* CommandList )
	{
		drn_check(OwningReflectionComponent);

		// TODO: add default black cubemap
		CubemapIndex = OwningReflectionComponent->GetCachedCubemap() ? OwningReflectionComponent->GetCachedCubemap()->GetShaderResourceView()->GetDescriptorHeapIndex() : 0;

		Position = OwningReflectionComponent->GetWorldLocation();
		InfluenceRadius = OwningReflectionComponent->GetInfluenceBoundingRadius();
		CaptureOffset = OwningReflectionComponent->CaptureOffset;
		Brightness = OwningReflectionComponent->Brightness;
	}

}