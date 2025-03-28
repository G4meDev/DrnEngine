#pragma once

#include "ForwardTypes.h"
#include "Asset.h"

#include "Runtime/Engine/StaticMesh.h"

LOG_DECLARE_CATEGORY(LogAssetManager)

namespace Drn
{
	class AssetManager;

	enum class EAssetType : uint8
	{
		StaticMesh = 0
	};

	template<typename T>
	struct AssetHandle
	{
	public:
		AssetHandle(const std::string& InPath)
			: m_Path(InPath)
			, m_Asset(nullptr) {}

		AssetHandle() 
			: AssetHandle("") {}

		~AssetHandle() 
		{
			Release();
		}

		AssetHandle& operator=(const AssetHandle& other)
		{
			this->m_Path = other.m_Path;
			this->m_Asset = other.m_Asset;

			if (this->m_Asset)
			{
				this->m_Asset->AddRef();
			}
		
			return *this;
		}

		void Load()
		{
			Release();
			m_Asset = AssetManager::Get()->Load<T>(m_Path);
			return;
		}

		void Release() 
		{
			if (IsValid())
			{
				m_Asset->RemoveRef();
			}
		}

		inline T* Get() { return m_Asset; }
		inline bool IsValid() { return m_Asset != nullptr; }

	private:
		T* m_Asset;
		std::string m_Path;
	};

	class AssetManager
	{
	public:

		static void Init();
		static void Shutdown();

		inline static AssetManager* Get() { return m_SingletionInstance; }

		template< typename T >
		T* Load(const std::string& Path);

	protected:

		std::unordered_map<std::string, Asset*> AssetRegistery;

		static AssetManager* m_SingletionInstance;

	private:
	};


	template<typename T>
	T* AssetManager::Load( const std::string& Path )
	{
		Asset* asset = nullptr;
		auto it = AssetRegistery.find(Path);

		if (it != AssetRegistery.end())
		{
			asset = it->second;
		}

		else
		{
			asset = new T(Path);
			AssetRegistery[Path] = asset;
		}

		asset->AddRef();
		return static_cast<T*>(asset);
	}

}