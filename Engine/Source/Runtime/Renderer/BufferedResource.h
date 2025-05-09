#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class BufferedResource
	{
	public:
		BufferedResource();
		virtual ~BufferedResource();

		void ReleaseBufferedResource();

	protected:
		bool ReleaseBufferedResourceConditional();
		void ReleaseBufferedResourceForced();
		uint8 m_RemainingLifeTime;

		friend class BufferedResourceManager;
	};

	class BufferedResourceManager
	{
	public:
		BufferedResourceManager();
		~BufferedResourceManager();

		static void Init();
		static void Shutdown();
		inline static BufferedResourceManager* Get() { return m_SingletonInstance; }
		
		void Tick(float DeltaTime);
		void Flush();

	protected:
		void RegisterPendingRelease(BufferedResource* Resource);
		friend class BufferedResource;

		std::set<BufferedResource*> m_PendingReleaseResources;
		static BufferedResourceManager* m_SingletonInstance;
	};

}