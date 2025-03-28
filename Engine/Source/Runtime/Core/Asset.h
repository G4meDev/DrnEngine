#pragma once

#include "ForwardTypes.h"

LOG_DECLARE_CATEGORY( LogAsset )

namespace Drn
{
	class Asset
	{
	public:
		Asset(const std::string InPath);


		uint16 RefCount;

		inline void AddRef() { RefCount++; }
		inline void RemoveRef() { RefCount--; }

	protected:

		virtual void Load() = 0;

		std::string m_Path;

		friend class AssetManager;
	private:

	};
}