#include "DrnPCH.h"
#include "LevelViewport.h"

#if WITH_EDITOR

#include "LevelViewportGuiLayer.h"

LOG_DEFINE_CATEGORY( LogLevelViewport, "LevelViewport" );

namespace Drn
{
	LevelViewport* LevelViewport::SingletonInstance = nullptr;

	LevelViewport::LevelViewport( World* InOwningWorld )
		: m_SelectedComponent(nullptr)
	{
		LevelViewportLayer = std::make_unique<LevelViewportGuiLayer>( this );
		LevelViewportLayer->Attach();

		m_OwningWorld = InOwningWorld;
		if (m_OwningWorld)
		{
			m_OwningWorld->OnRemoveActors.Add( this, &LevelViewport::OnRemovedActorsFromWorld);
		}
	}

	LevelViewport::~LevelViewport()
	{
		LevelViewportLayer->DeAttach();
		LevelViewportLayer.reset();

		if (m_OwningWorld)
		{
			m_OwningWorld->OnRemoveActors.Remove( this );
		}
	}

	void LevelViewport::Init( World* InOwningWorld )
	{
		Shutdown();

		if (InOwningWorld)
		{
			SingletonInstance = new LevelViewport( InOwningWorld );
		}
	}

	void LevelViewport::Shutdown()
	{
		if (SingletonInstance)
		{
			delete SingletonInstance;
			SingletonInstance = nullptr;
		}
	}

	void LevelViewport::Tick( float DeltaTime )
	{
		Actor* SelectedActor = m_SelectedComponent ? m_SelectedComponent->GetOwningActor() : nullptr;

		if (SelectedActor)
		{
			SelectedActor->DrawEditorSelected();
		}
	}

	LevelViewport* LevelViewport::Get()
	{
		return SingletonInstance;
	}

	void LevelViewport::OnSelectedNewComponent(const HitProxyData& Data)
	{
		Component* OldComponent = m_SelectedComponent;
		Component* NewComponent = m_OwningWorld->GetComponentWithID(Data.ComponentID);

		if ( NewComponent == nullptr )
		{
			m_SelectedComponent = nullptr;
		}

		else if (m_SelectedComponent && m_SelectedComponent->GetOwningActor() == NewComponent->GetOwningActor())
		{
			m_SelectedComponent = NewComponent;
		}

		else
		{
			m_SelectedComponent = NewComponent->GetOwningActor()->GetRoot();
		}

		if (OldComponent && OldComponent != NewComponent && OldComponent->GetOwningActor())
		{
			//OldComponent->SetSelectedInEditor(false);
			OldComponent->GetOwningActor()->SetComponentsSelectedInEditor(false, Data);
		}

		if (m_SelectedComponent && m_SelectedComponent->GetOwningActor())
		{
			//m_SelectedComponent->SetSelectedInEditor(true);
			m_SelectedComponent->GetOwningActor()->SetComponentsSelectedInEditor(true, Data);
		}
	}

	void LevelViewport::GetGizmoTransform( bool& bDrawGizmo, Transform& GizmoTransform )
	{
		SceneComponent* SceneComp = dynamic_cast<SceneComponent*>(m_SelectedComponent);

		bDrawGizmo = !m_OwningWorld->IsInGameMode() && SceneComp && SceneComp->GetOwningActor() &&
			!SceneComp->GetOwningActor()->IsMarkedPendingKill();

		if (bDrawGizmo)
		{
			GizmoTransform = SceneComp->GetGizmoTransform();
		}
	}

	void LevelViewport::OnGizmoTransformChanged( const Transform& GizmoTransform, EGizmoSpace GizmoSpace )
	{
		SceneComponent* SceneComp = dynamic_cast<SceneComponent*>(m_SelectedComponent);
		
		if (SceneComp)
		{
			SceneComp->OnGizmoTransformChanged(GizmoTransform, GizmoSpace);
		}
	}

	void LevelViewport::OnRemovedActorsFromWorld( std::vector<Actor*> RemovedActors )
	{
		if (m_SelectedComponent)
		{
			for (Actor* actor : RemovedActors)
			{
				if (actor == m_SelectedComponent->GetOwningActor())
				{
					m_SelectedComponent = nullptr;
					break;
				}
			}
		}


	}

}

#endif