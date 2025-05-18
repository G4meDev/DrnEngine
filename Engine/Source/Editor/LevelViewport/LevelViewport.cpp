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
		
	}

	LevelViewport* LevelViewport::Get()
	{
		return SingletonInstance;
	}

	void LevelViewport::OnSelectedNewComponent( Component* NewComponent )
	{
		Component* OldComponent = m_SelectedComponent;

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

		if (OldComponent)
		{
			OldComponent->SetSelectedInEditor(false);
		}

		if (m_SelectedComponent)
		{
			m_SelectedComponent->SetSelectedInEditor(true);
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