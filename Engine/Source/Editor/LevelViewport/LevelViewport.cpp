#include "DrnPCH.h"
#include "LevelViewport.h"

#if WITH_EDITOR

#include "LevelViewportGuiLayer.h"

LOG_DEFINE_CATEGORY( LogLevelViewport, "LevelViewport" );

namespace Drn
{
	std::unique_ptr<Drn::LevelViewport> LevelViewport::SingletonInstance;

	LevelViewport::LevelViewport()
		: m_SelectedComponent(nullptr)
	{
	}

	LevelViewport::~LevelViewport()
	{
	}

	void LevelViewport::Init()
	{
		LevelViewportLayer = std::make_unique<LevelViewportGuiLayer>( this );
		LevelViewportLayer->Attach();
	}

	void LevelViewport::Shutdown()
	{
		LevelViewportLayer->DeAttach();
		LevelViewportLayer.reset();
	}

	void LevelViewport::Tick( float DeltaTime )
	{
		
	}

	LevelViewport* LevelViewport::Get()
	{
		if ( !SingletonInstance )
		{
			SingletonInstance = std::make_unique<LevelViewport>();
		}

		return SingletonInstance.get();
	}

	void LevelViewport::OnSelectedNewComponent( Component* NewComponent )
	{
		m_SelectedComponent = NewComponent;
		std::cout << "??????????????????????????????????????????????????\n";
	}

}

#endif