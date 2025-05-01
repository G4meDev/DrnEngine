#pragma once

#include "ForwardTypes.h"
#include "Asset.h"

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
			if (other.IsValid())
			{
				other.m_Asset->AddRef();
			}

			if (this->IsValid())
			{
				this->Release();
			}

			this->m_Path = other.m_Path;
			this->m_Asset = other.m_Asset;

			return *this;
		}

		inline std::string GetPath() { return m_Path; }

		void Load()
		{
			Release();
			m_Asset = AssetManager::Get()->Load<T>(m_Path);
			return;
		}

		EAssetType LoadGeneric()
		{
			Release();

			uint16 TypeByte;
			{
				Archive Ar(Path::ConvertProjectPath(m_Path));
				Ar >> TypeByte;
			}

			EAssetType Type = static_cast<EAssetType>(TypeByte);

			if (Type == EAssetType::StaticMesh)
			{
				m_Asset = AssetManager::Get()->Load<StaticMesh>(m_Path);
			}

			else if (Type == EAssetType::Level)
			{
				m_Asset = AssetManager::Get()->Load<Level>(m_Path);
			}

			else if (Type == EAssetType::Material)
			{
				m_Asset = AssetManager::Get()->Load<Material>(m_Path);
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
		inline bool IsValid() const { return m_Asset != nullptr; }

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

		template <typename T>
		void Create(const std::string& TargetFolder, const std::string& AssetNamePrefix, int Unused);

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


#if WITH_EDITOR
	template<typename T>
	void AssetManager::Create( const std::string& TargetFolder, const std::string& AssetNamePrefix, int Unused)
	{
		std::string TargetFolderFullPath = Path::ConvertProjectPath(TargetFolder);

		std::string NewAssetName;
		if (Path::GetNameForNewAsset(TargetFolderFullPath, AssetNamePrefix, NewAssetName))
		{
			std::unique_ptr<T> NewAsset = std::make_unique<T>(TargetFolder + "\\" + NewAssetName, "");
		}
	}
#endif

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