#pragma once

#include "ForwardTypes.h"
#include "Asset.h"

#include "Runtime/Engine/StaticMesh.h"

LOG_DECLARE_CATEGORY(LogAssetManager)

namespace Drn
{
	class AssetManager;

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

		T* operator*()
		{
			return m_Asset;
		}

		T* operator->() 
		{
			return m_Asset;
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

		EAssetType LoadGeneric()
		{
			Release();

			uint8 TypeByte;
			{
				Archive Ar(m_Path);
				Ar >> TypeByte;
			}

			EAssetType Type = static_cast<EAssetType>(TypeByte);

			if (Type == EAssetType::StaticMesh)
			{
				m_Asset = AssetManager::Get()->Load<StaticMesh>(m_Path);
			}

			return Type;
		}

		void Release() 
		{
			if (IsValid())
			{
				m_Asset->RemoveRef();

				if (m_Asset->m_RefCount <= 0)
				{
					AssetManager::Get()->InvalidateAsset(m_Asset);
				}
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

		void ReportLiveAssets();

		template< typename T >
		T* Load(const std::string& Path);

#if WITH_EDITOR
		void Create(const std::string& SourceFile, const std::string& TargetFolder);
#endif

		template< typename T >
		void InvalidateAsset(T*& InAsset);

	protected:

		std::unordered_map<std::string, Asset*> m_AssetRegistery;

		static AssetManager* m_SingletionInstance;

	private:
	};


	template<typename T>
	T* AssetManager::Load( const std::string& Path )
	{
		Asset* asset = nullptr;
		auto it = m_AssetRegistery.find(Path);

		if (it != m_AssetRegistery.end())
		{
			asset = it->second;
		}

		else
		{
			asset = new T(Path);
			m_AssetRegistery[Path] = asset;
		}

		asset->AddRef();
		return static_cast<T*>(asset);
	}

	template<typename T>
	void AssetManager::InvalidateAsset(T*& InAsset)
	{
		auto it = m_AssetRegistery.find( InAsset->m_Path );

		if ( it != m_AssetRegistery.end() )
		{
			m_AssetRegistery.erase( it );
		}

		delete InAsset;
		InAsset = nullptr;
	}

}