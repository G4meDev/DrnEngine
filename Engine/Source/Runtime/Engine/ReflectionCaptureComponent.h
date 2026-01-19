#pragma once

#include "ForwardTypes.h"
#include "Runtime/Engine/SceneComponent.h"

namespace Drn
{
	class RenderTextureCube;
	class ReflectionCaptureProxy;

	class ReflectionCaptureComponent : public SceneComponent
	{
	public:

		ReflectionCaptureProxy* SceneProxy;

		virtual float GetInfluenceBoundingRadius() const = 0;

		inline TRefCountPtr<RenderTextureCube>& GetCachedCubemap() { return CachedCubemap; }
		inline ReflectionCaptureData& GetCachedData() { return CachedData; }

		inline float GetMaxCaptureDistance() const { return MaxCaptureDistance; }
		inline void SetMaxCaptureDistance(float InMaxCaptureDistance) { MaxCaptureDistance = MaxCaptureDistance; }

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

		// mainly to cull sky
		float MaxCaptureDistance;

		ReflectionCaptureData CachedData;

	private:

		TRefCountPtr<RenderTextureCube> CachedCubemap;

		friend class ReflectionCaptureProxy;
	};
}