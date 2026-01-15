#include "DrnPCH.h"
#include "ReflectionCaptureComponent.h"

namespace Drn
{
	ReflectionCaptureComponent::ReflectionCaptureComponent()
		: SceneComponent()
		, Brightness(1.0f)
		, CaptureOffset(Vector::ZeroVector)
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
		}
		else
		{
			Ar << Brightness;
			Ar << CaptureOffset;
		}
	}

	void ReflectionCaptureComponent::RegisterComponent( World* InOwningWorld )
	{
		SceneComponent::RegisterComponent(InOwningWorld);

	}

	void ReflectionCaptureComponent::UnRegisterComponent()
	{
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

		ImGui::DragFloat("Brightness", &Brightness, 0.1f, 0.5f, 4.0f);
		CaptureOffset.Draw("CaptureOffset");
	}

	void ReflectionCaptureComponent::SetSelectedInEditor( bool SelectedInEditor )
	{
		SceneComponent::SetSelectedInEditor(SelectedInEditor);
	}
#endif
}  // namespace Drn