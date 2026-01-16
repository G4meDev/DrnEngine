#pragma once

#include "ForwardTypes.h"
#include "Runtime/Engine/SceneComponent.h"

namespace Drn
{
	class RenderTextureCube;

	class ReflectionCaptureComponent : public SceneComponent
	{
	public:

		//ReflectionCaptureProxy* SceneProxy;
		//ReflectionCaptureProxy* CreateSceneProxy();

		//virtual float GetInfluenceBoundingRadius() const;

		/** Called each tick to recapture and queued reflection captures. */
		//static void UpdateReflectionCaptureContents(UWorld* WorldToUpdate, const TCHAR* CaptureReason = nullptr, bool bVerifyOnlyCapturing = false, bool bCapturingForMobile = false);

		//static int32 GetReflectionCaptureSize();

#if WITH_EDITOR
		void MarkNeedRecapture() { bNeedsRecapture = true; ReflectionCapturesToUpdate.insert(this); }
		void ClearNeedRecapture() { bNeedsRecapture = false; }
		bool NeedsRecapture() const { return bNeedsRecapture; }

		static std::set<ReflectionCaptureComponent*>& GetReflectionCapturesToUpdate() { return ReflectionCapturesToUpdate; }
#endif

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

		bool bNeedsRecapture = false;
		static std::set<ReflectionCaptureComponent*> ReflectionCapturesToUpdate;
#endif

		float Brightness;
		Vector CaptureOffset;

		ReflectionCaptureData CachedData;

	private:

		TRefCountPtr<RenderTextureCube> CachedCubemap;


		friend class ReflectionCaptureProxy;
	};
}