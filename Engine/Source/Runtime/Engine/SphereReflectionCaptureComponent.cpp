#include "DrnPCH.h"
#include "SphereReflectionCaptureComponent.h"

#if WITH_EDITOR
#include <imgui.h>
#endif

namespace Drn
{
	SphereReflectionCaptureComponent::SphereReflectionCaptureComponent()
		: ReflectionCaptureComponent()
		, InfluenceRadius(10.0f)
	{
		
	}

	SphereReflectionCaptureComponent::~SphereReflectionCaptureComponent()
	{
		
	}

	void SphereReflectionCaptureComponent::Serialize( Archive& Ar )
	{
		ReflectionCaptureComponent::Serialize(Ar);

		if (Ar.IsLoading())
		{
			Ar >> InfluenceRadius;
		}
		else
		{
			Ar << InfluenceRadius;
		}
	}


	void SphereReflectionCaptureComponent::RegisterComponent( World* InOwningWorld )
	{
		ReflectionCaptureComponent::RegisterComponent(InOwningWorld);

#if WITH_EDITOR
		AssetHandle<Texture2D> ReflectionCaptureIcon( "Engine\\Content\\EditorResources\\T_DefaultComponentIcon.drn" );
		ReflectionCaptureIcon.Load();

		m_Sprite->SetSprite( ReflectionCaptureIcon );
#endif
	}

	void SphereReflectionCaptureComponent::UnRegisterComponent()
	{
		ReflectionCaptureComponent::UnRegisterComponent();
	}

	void SphereReflectionCaptureComponent::OnUpdateTransform( bool SkipPhysic )
	{
		ReflectionCaptureComponent::OnUpdateTransform(SkipPhysic);

		SetRelativeRotation(Quat::Identity);
		SetRelativeScale(Vector::OneVector);
	}

#if WITH_EDITOR
	void SphereReflectionCaptureComponent::DrawDetailPanel( float DeltaTime )
	{
		ReflectionCaptureComponent::DrawDetailPanel(DeltaTime);

		ImGui::DragFloat("InfluenceRadius", &InfluenceRadius, 1.0f, 0.1f, 16384.0f);
	}

	void SphereReflectionCaptureComponent::DrawAttenuation()
	{
		if (GetWorld())
		{
			GetWorld()->DrawDebugSphere( GetWorldLocation(), Quat::Identity, Color::White, InfluenceRadius, 36, 0.0, 0 );
			GetWorld()->DrawDebugSphere( GetWorldLocation() + CaptureOffset, Quat::Identity, Color::Blue, 0.5f, 12, 0.0, 0 );
		}
	}

	void SphereReflectionCaptureComponent::DrawEditorDefault()
	{
		ReflectionCaptureComponent::DrawEditorDefault();

	}

	void SphereReflectionCaptureComponent::DrawEditorSelected()
	{
		ReflectionCaptureComponent::DrawEditorSelected();
		
		DrawAttenuation();
	}
#endif
}  // namespace Drn