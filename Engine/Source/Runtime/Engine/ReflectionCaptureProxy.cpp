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

		RenderTextureCube* TargetCubemap = OwningReflectionComponent->GetCachedCubemap() ? OwningReflectionComponent->GetCachedCubemap() : CommonResources::Get()->m_BlackCubemap;
		CubemapIndex = TargetCubemap->GetShaderResourceView()->GetDescriptorHeapIndex();
		Position = OwningReflectionComponent->GetWorldLocation();
		InfluenceRadius = OwningReflectionComponent->GetInfluenceBoundingRadius();
		CaptureOffset = OwningReflectionComponent->CaptureOffset;
		Brightness = OwningReflectionComponent->Brightness;
	}

}