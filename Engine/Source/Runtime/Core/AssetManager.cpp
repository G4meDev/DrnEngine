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
		if (m_SingletionInstance)
		{
			m_SingletionInstance->ReportLiveAssets();

			delete m_SingletionInstance;
			m_SingletionInstance = nullptr;
		}

	}

	void AssetManager::ReportLiveAssets()
	{
		OutputDebugString("Asset Manager: reporting live assets.\n");
		LOG(LogAssetManager, Info, "reprting live assets.");

		for (auto& it : m_AssetRegistery)
		{
			char Msg[500];
			sprintf(Msg, "\t(%u) %s\n", it.second->RefCount, it.first.c_str());
			OutputDebugString( Msg );

			LOG(LogAssetManager, Info, "\t(%u) %s", it.second->RefCount, it.first.c_str());
		}
	}


}