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

		// @TODO: update on state dirty
		OwningReflectionComponent->UploadCachedReflectionCapture(CommandList);

		RenderTextureCube* TargetCubemap = OwningReflectionComponent->GetCachedCubemap() ? OwningReflectionComponent->GetCachedCubemap() : CommonResources::Get()->m_BlackCubemap;
		CubemapIndex = TargetCubemap->GetShaderResourceView()->GetDescriptorHeapIndex();
		Position = OwningReflectionComponent->GetWorldLocation();
		InfluenceRadius = OwningReflectionComponent->GetInfluenceBoundingRadius();
		CaptureOffset = OwningReflectionComponent->CaptureOffset;
		Brightness = OwningReflectionComponent->Brightness;
	}

	bool ReflectionCaptureProxy::HasValidCubemap() const
	{
		return OwningReflectionComponent->GetCachedCubemap().IsValid();
	}

	RenderTextureCube* ReflectionCaptureProxy::GetCubemap() const
	{
		return OwningReflectionComponent->GetCachedCubemap() ? OwningReflectionComponent->GetCachedCubemap() : CommonResources::Get()->m_BlackCubemap;
	}

	Vector ReflectionCaptureProxy::GetPosition() const
	{
		return OwningReflectionComponent->GetWorldLocation();
	}

	float ReflectionCaptureProxy::GetRadius() const
	{
		return OwningReflectionComponent->GetInfluenceBoundingRadius();
	}

	Vector ReflectionCaptureProxy::GetCaptureOffset() const
	{
		return OwningReflectionComponent->CaptureOffset;
	}

	float ReflectionCaptureProxy::GetBrightness() const
	{
		return OwningReflectionComponent->Brightness;
	}

}  // namespace Drn