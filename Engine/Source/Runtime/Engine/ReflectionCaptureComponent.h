#pragma once

#include "ForwardTypes.h"
#include "Runtime/Engine/SceneComponent.h"

namespace Drn
{
	class RenderTextureCube;

	class ReflectionCaptureComponent : public SceneComponent
	{
		//ReflectionCaptureProxy* SceneProxy;

		//ReflectionCaptureProxy* CreateSceneProxy();

		//void MarkDirtyForRecaptureOrUpload();

		//void MarkDirtyForRecapture();

		//void SetCaptureCompleted() { bNeedsRecaptureOrUpload = false; }

		//virtual float GetInfluenceBoundingRadius() const;

		/** Called each tick to recapture and queued reflection captures. */
		//static void UpdateReflectionCaptureContents(UWorld* WorldToUpdate, const TCHAR* CaptureReason = nullptr, bool bVerifyOnlyCapturing = false, bool bCapturingForMobile = false);

		//static int32 GetReflectionCaptureSize();

	protected:
		ReflectionCaptureComponent();
		virtual ~ReflectionCaptureComponent();

		virtual void Serialize( Archive& Ar ) override;

		virtual void RegisterComponent(World* InOwningWorld) override;
		virtual void UnRegisterComponent() override;

		virtual void OnUpdateTransform( bool SkipPhysic ) override;

#if WITH_EDITOR
		virtual void DrawDetailPanel(float DeltaTime) override;
		virtual void SetSelectedInEditor( bool SelectedInEditor ) override;
#endif

		float Brightness;
		Vector CaptureOffset;

	private:

		//bool bNeedsRecaptureOrUpload;

		//RenderTextureCube* CachedEncodedHDRCubemap;

		//static TArray<UReflectionCaptureComponent*> ReflectionCapturesToUpdate;

		//static TArray<UReflectionCaptureComponent*> ReflectionCapturesToUpdateForLoad;

		friend class ReflectionCaptureProxy;
	};
}