#include "DrnPCH.h"
#include "BufferedResource.h"

namespace Drn
{
	BufferedResourceManager* BufferedResourceManager::m_SingletonInstance = nullptr;

	BufferedResource::BufferedResource()
		: m_RemainingLifeTime(NUM_BACKBUFFERS)
	{
	}

	BufferedResource::~BufferedResource()
	{
	}

	void BufferedResource::ReleaseBufferedResource()
	{
		if (m_RemainingLifeTime > 0)
		{
			BufferedResourceManager::Get()->RegisterPendingRelease(this);
		}

		else
		{
			ReleaseBufferedResourceForced();
		}
	}

	bool BufferedResource::ReleaseBufferedResourceConditional()
	{
		if (--m_RemainingLifeTime == 0)
		{
			delete this;
			return true;
		}

		return false;
	}

	void BufferedResource::ReleaseBufferedResourceForced()
	{
		delete this;
	}

	BufferedResourceManager::BufferedResourceManager()
	{
	}

	BufferedResourceManager::~BufferedResourceManager()
	{
	}

	void BufferedResourceManager::Init()
	{
		if (!m_SingletonInstance)
		{
			m_SingletonInstance = new BufferedResourceManager();
		}
	}

	void BufferedResourceManager::Shutdown()
	{
		if (m_SingletonInstance)
		{
			delete m_SingletonInstance;
		}
	}

	void BufferedResourceManager::Tick( float DeltaTime )
	{
		SCOPE_STAT( BufferedResourceManagerTick );

		for (auto it = m_PendingReleaseResources.begin(); it != m_PendingReleaseResources.end(); )
		{
			BufferedResource* Resource = *it;

			if (Resource->ReleaseBufferedResourceConditional())
			{
				it = m_PendingReleaseResources.erase(it);
			}

			else
			{
				it++;
			}
		}
	}

	void BufferedResourceManager::Flush()
	{
		for (BufferedResource* Resource : m_PendingReleaseResources)
		{
			Resource->ReleaseBufferedResourceForced();
		}

		m_PendingReleaseResources.clear();
	}

	void BufferedResourceManager::RegisterPendingRelease( BufferedResource* Resource )
	{
		if (Resource)
		{
			m_PendingReleaseResources.insert(Resource);
		}
	}
}