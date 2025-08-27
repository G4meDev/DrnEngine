#include "DrnPCH.h"
#include "CameraManager.h"

namespace Drn
{
	CameraManager::CameraManager()
		: Actor()
		, m_ViewTarget(nullptr)
	{
		m_RootComponent = std::make_unique<class SceneComponent>();
		m_RootComponent->SetComponentLabel( "RootComponent" );
		SetRootComponent(m_RootComponent.get());

		SetTransient(true);
	}

	CameraManager::~CameraManager()
	{
		
	}

	void CameraManager::Serialize( Archive& Ar )
	{
		// Nothing. transient
	}

	void CameraManager::Tick( float DeltaTime )
	{
		Actor::Tick(DeltaTime);

		if (m_ViewTarget)
		{
			m_ViewTarget->CalcCamera(m_ViewInfo);
		}
	}

	void CameraManager::SetViewTarget( Actor* Target )
	{
		if (m_ViewTarget && Target != m_ViewTarget)
		{
			m_ViewTarget->OnActorKilled.Remove(this);
		}

		m_ViewTarget = (Target && !Target->IsMarkedPendingKill()) ? Target : nullptr;
		if (m_ViewTarget)
		{
			m_ViewTarget->OnActorKilled.Add(this, &CameraManager::OnViewTargetDestroyed);
		}
	}

	void CameraManager::OnViewTargetDestroyed( Actor* Target )
	{
		if (Target == m_ViewTarget)
		{
			m_ViewTarget = nullptr;
		}
	}

}