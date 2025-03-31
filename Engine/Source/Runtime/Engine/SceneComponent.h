#pragma once

#include "ForwardTypes.h"
#include "Component.h"

namespace Drn
{
	class SceneComponent : public Component
	{
	public:
		SceneComponent();
		
		virtual void Tick(float DeltaTime) override;

		void AttachSceneComponent(SceneComponent* InComponent);

		const std::vector<SceneComponent*>& GetChilds() const;

		// ------------------ location -----------------

		DirectX::XMVECTOR GetRelativeLocation() const;
		DirectX::XMVECTOR GetLocalLocation() const;
		DirectX::XMVECTOR GetWorldLocation() const;

		void SetRelativeLocation(const DirectX::XMVECTOR& Inlocation, bool bMarkDirty = true);
		void SetLocalLocation(const DirectX::XMVECTOR& Inlocation, bool bMarkDirty = true);
		void SetWorldLocation(const DirectX::XMVECTOR& Inlocation, bool bMarkDirty = true);

		void UpdateLocation();

		bool IsDirtyLocation() const;
		void MarkDirtyLocation();
		void MarkDirtyLocationRecursive();

		// ------------------ rotation --------------------

/*
		const Quaternion GetRelativeRotation() const;
		const Quaternion GetLocalRotation() const;
		const Quaternion GetWorldRotation() const;

		void SetRelativeRotation(const Quaternion& InRotator, bool bMarkDirty = true);
		void SetLocalRotation(const Quaternion& InRotator, bool bMarkDirty = true);
		void SetWorldRotation(const Quaternion& InRotator, bool bMarkDirty = true);

		void UpdateRotation();

		bool IsDirtyRotation() const;
		void MarkDirtyRotation();
		void MarkDirtyRotationRecursive();
*/

		// ------------------ scale ------------------------

/*
		const Vector3 GetRelativeScale() const;
		const Vector3 GetLocalScale() const;
		const Vector3 GetWorldScale() const;

		void SetRelativeScale(const Vector3& InScale, bool bMarkDirty = true);
		void SetLocalScale(const Vector3& InScale, bool bMarkDirty = true);
		void SetWorldScale(const Vector3& InScale, bool bMarkDirty = true);

		void UpdateScale();

		bool IsDirtyScale() const;
		void MarkDirtyScale();
		void MarkDirtyScaleRecursive();
*/

	private:
		
		SceneComponent* Parent = nullptr;
		std::vector<SceneComponent*> Childs;

		DirectX::XMVECTOR WorldLocation;
		DirectX::XMVECTOR RelativeLocation;
		DirectX::XMVECTOR LocalLocation;

/*
		Quaternion WorldRotation	= Quaternion::Identity;
		Quaternion RelativeRotation	= Quaternion::Identity;
		Quaternion LocalRotation	= Quaternion::Identity;

		Vector3 WorldScale			= Vector3::Zero;
		Vector3 RelativeScale		= Vector3::Zero;
		Vector3 LocalScale			= Vector3::Zero;
*/

		bool bDirtyLocation = true;
		bool bDirtyRotation = true;
		bool bDirtyScale	= true;

	};
}