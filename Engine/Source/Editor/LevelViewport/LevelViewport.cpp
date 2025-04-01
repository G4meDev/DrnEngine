#include "DrnPCH.h"
#include "LevelViewport.h"

#if WITH_EDITOR

#include "LevelViewportGuiLayer.h"

namespace Drn
{
	std::unique_ptr<Drn::LevelViewport> LevelViewport::SingletonInstance;

	LevelViewport::LevelViewport()
	{
		
	}

	LevelViewport::~LevelViewport()
	{
		
	}

	void LevelViewport::Init()
	{
		LevelViewportLayer = std::make_unique<LevelViewportGuiLayer>();
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
}

#endif