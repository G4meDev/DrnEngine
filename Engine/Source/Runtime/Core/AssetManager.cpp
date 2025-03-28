#include "DrnPCH.h"
#include "AssetManager.h"

LOG_DEFINE_CATEGORY( LogAssetManager, "AssetManager" )

namespace Drn
{
	AssetManager* AssetManager::m_SingletionInstance;

	void AssetManager::Init()
	{
		m_SingletionInstance = new AssetManager();
	}

	void AssetManager::Shutdown()
	{
		delete m_SingletionInstance;
	}
}

// Result = new StaticMesh*;
//*Result = new StaticMesh(Path);

// Result = new StaticMesh*;
//*Result = NewAsset;