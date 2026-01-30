#include "DrnPCH.h"
#include "ReflectionCaptureComponent.h"
#include "Runtime/Engine/ReflectionCaptureProxy.h"

namespace Drn
{
#if WITH_EDITOR
	std::set<ReflectionCaptureComponent*> ReflectionCaptureComponent::ReflectionCapturesToUpdate;
#endif

	ReflectionCaptureComponent::ReflectionCaptureComponent()
		: SceneComponent()
		, SceneProxy(nullptr)
		, Brightness(1.0f)
		, CaptureOffset(Vector::ZeroVector)
		, MaxCaptureDistance(100.0f)
	{
		
	}

	ReflectionCaptureComponent::~ReflectionCaptureComponent()
	{
		
	}

	void ReflectionCaptureComponent::Serialize( Archive& Ar )
	{
		SceneComponent::Serialize(Ar);

		if (Ar.IsLoading())
		{
			Ar >> Brightness;
			Ar >> CaptureOffset;
			Ar >> MaxCaptureDistance;

			Ar >> CachedData;
		}
		else
		{
			Ar << Brightness;
			Ar << CaptureOffset;
			Ar << MaxCaptureDistance;

			Ar << CachedData;
		}
	}

	void ReflectionCaptureComponent::RegisterComponent( World* InOwningWorld )
	{
		SceneComponent::RegisterComponent(InOwningWorld);

		SceneProxy = new ReflectionCaptureProxy( this );
		InOwningWorld->GetScene()->RegisterReflectionCaptureProxy(SceneProxy);
	}

	void ReflectionCaptureComponent::UnRegisterComponent()
	{
		if (SceneProxy)
		{
			SceneProxy->MarkPendingDestroy();
		}

		SceneComponent::UnRegisterComponent();
	}

	void ReflectionCaptureComponent::OnUpdateTransform( bool SkipPhysic )
	{
		SceneComponent::OnUpdateTransform(SkipPhysic);

	}

#if WITH_EDITOR
	void ReflectionCaptureComponent::DrawDetailPanel( float DeltaTime )
	{
		SceneComponent::DrawDetailPanel(DeltaTime);

		if (ImGui::Button("Capture"))
		{
			MarkNeedRecapture();
		}

		ImGui::DragFloat("Brightness", &Brightness, 0.1f, 0.5f, 4.0f);
		CaptureOffset.Draw("CaptureOffset");

		ImGui::DragFloat("MaxCaptureDistance", &MaxCaptureDistance, 1.0f, 1.0f, 10000.0f);
	}

	void ReflectionCaptureComponent::SetSelectedInEditor( bool SelectedInEditor )
	{
		SceneComponent::SetSelectedInEditor(SelectedInEditor);
	}

#endif
}  // namespace Drn