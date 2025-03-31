#pragma once

#include "ForwardTypes.h"
#include "Component.h"

namespace Drn
{
	class SceneComponent : public Component
	{
	public:
		SceneComponent();
		virtual ~SceneComponent();
		
		virtual void Tick(float DeltaTime) override;

		inline virtual EComponentType GetComponentType() override { return EComponentType::SceneComponenet; }

		void AttachSceneComponent(SceneComponent* InComponent);

		const std::vector<std::shared_ptr<SceneComponent>>& GetChilds() const;

		template<typename T>
		void GetComponents(std::vector<T*>& OutComponents, EComponentType Type, bool Recursive)
		{
			for (auto Comp : Childs)
			{
				if (Comp->GetComponentType() == Type)
				{
					OutComponents.push_back( static_cast<T*>(Comp.get()) );

					if (Recursive)
					{
						Comp->GetComponents(OutComponents, Type, true);
					}
				}
			}
		}

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

		DirectX::XMVECTOR GetRelativeRotation() const;
		DirectX::XMVECTOR GetLocalRotation() const;
		DirectX::XMVECTOR GetWorldRotation() const;

		void SetRelativeRotation(const DirectX::XMVECTOR& InRotator, bool bMarkDirty = true);
		void SetLocalRotation(const DirectX::XMVECTOR& InRotator, bool bMarkDirty = true);
		void SetWorldRotation(const DirectX::XMVECTOR& InRotator, bool bMarkDirty = true);

		void UpdateRotation();

		bool IsDirtyRotation() const;
		void MarkDirtyRotation();
		void MarkDirtyRotationRecursive();

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
		std::vector<std::shared_ptr<SceneComponent>> Childs;

		DirectX::XMVECTOR WorldLocation;
		DirectX::XMVECTOR RelativeLocation;
		DirectX::XMVECTOR LocalLocation;

		DirectX::XMVECTOR WorldRotation;
		DirectX::XMVECTOR RelativeRotation;
		DirectX::XMVECTOR LocalRotation;

/*
		Vector3 WorldScale			= Vector3::Zero;
		Vector3 RelativeScale		= Vector3::Zero;
		Vector3 LocalScale			= Vector3::Zero;
*/

		bool bDirtyLocation = true;
		bool bDirtyRotation = true;
		bool bDirtyScale	= true;

	};
}