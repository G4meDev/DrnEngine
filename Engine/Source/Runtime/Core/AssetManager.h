#pragma once

#include "ForwardTypes.h"
#include "Asset.h"

#include "Runtime/Renderer/D3D12Utils.h"
#include "Runtime/Misc/Path.h"
#include "Runtime/Core/Archive.h"

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
			: AssetHandle("InvalidPath") {}

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

		bool operator==(const AssetHandle<T>& Other) const
		{
			return m_Path == Other.m_Path;
		}


		AssetHandle(const AssetHandle& other)
		{
			m_Asset = nullptr;

			this->m_Path = other.m_Path;
			if (other.IsValid())
			{
				other.m_Asset->AddRef();
				this->m_Asset = other.m_Asset;
			}
		}

		AssetHandle& operator=(const AssetHandle& other)
		{
			if (this->IsValid())
			{
				this->Release();
			}

			if (other.IsValid())
			{
				other.m_Asset->AddRef();
				this->m_Asset = other.m_Asset;
			}

			this->m_Path = other.m_Path;

			return *this;
		}

		inline std::string GetPath() const { return m_Path; }

		void Load()
		{
			Release();
			m_Asset = AssetManager::Get()->Load<T>(m_Path);
		}

		void LoadChecked()
		{
			Release();
			m_Asset = AssetManager::Get()->LoadChecked<T>(m_Path);
		}

		// TODO: mark this editor only
		EAssetType LoadGeneric()
		{
			Release();

			Archive Ar(Path::ConvertProjectPath(m_Path));
			if (!Ar.IsValid())
			{
				return EAssetType::Undefined;
			}

			uint16 TypeByte;
			Ar >> TypeByte;

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

			else if (Type == EAssetType::Texture2D)
			{
				m_Asset = AssetManager::Get()->Load<Texture2D>(m_Path);
			}

			// TODO: uncomment when defined asset types
			//else if (Type == EAssetType::TextureVolume)
			//{
			//	m_Asset = AssetManager::Get()->Load<Texture2D>(m_Path);
			//}
			//
			//else if (Type == EAssetType::TextureCube)
			//{
			//	m_Asset = AssetManager::Get()->Load<TextureCube>(m_Path);
			//}

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

				m_Asset = nullptr;
			}
		}

		//inline void ReleaseDeferred()
		//{
		//	AssetManager::Get()->RegisterPendingReleaseDeferred(*this);
		//}

		inline T* Get() { return m_Asset; }
		inline bool IsValid() const { return m_Asset != nullptr; }

	private:
		T* m_Asset;
		std::string m_Path;
	};

	struct StringHasher
	{
		size_t operator()(const AssetHandle<Asset>& t) const
		{
			return std::hash<std::string>()(t.GetPath());
		}
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

		template< typename T >
		T* LoadChecked(const std::string& Path);

#if WITH_EDITOR

		void Create(const std::string& SourceFile, const std::string& TargetFolder);

		template <typename T>
		void Create(const std::string& TargetFolder, const std::string& AssetNamePrefix, int Unused);

#endif

		template< typename T >
		void InvalidateAsset(T*& InAsset);

		//template< typename T >
		//void RegisterPendingReleaseDeferred(AssetHandle<T> Handle)
		//{
		//	if (Handle.IsValid())
		//	{
		//		// TODO: improve
		//		AssetHandle<Asset> assetHandle(Handle.GetPath());
		//		assetHandle.LoadGeneric();
		//		auto it = m_PendingReleaseDefferred.find(assetHandle);
		//
		//		if (it != m_PendingReleaseDefferred.end())
		//		{
		//			it->second = NUM_BACKBUFFERS;
		//		}
		//
		//		else
		//		{
		//			m_PendingReleaseDefferred[assetHandle] = NUM_BACKBUFFERS;
		//		}
		//	}
		//}

		//void Tick()
		//{
		//	for (auto it = m_PendingReleaseDefferred.begin(); it != m_PendingReleaseDefferred.end(); )
		//	{
		//		(it->second)--;
		//		if (it->second == 0)
		//		{
		//			it = m_PendingReleaseDefferred.erase(it);
		//		}
		//
		//		else
		//		{
		//			it++;
		//		}
		//	}
		//}

	protected:

		std::unordered_map<std::string, Asset*> m_AssetRegistery;
		//std::unordered_map<AssetHandle<Asset>, uint16, StringHasher> m_PendingReleaseDefferred;

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
	T* AssetManager::LoadChecked( const std::string& Path )
	{
		Asset* asset = nullptr;
		auto it = m_AssetRegistery.find(Path);

		if (it != m_AssetRegistery.end())
		{
			asset = it->second;

			asset->AddRef();
			return static_cast<T*>(asset);
		}

		else
		{
			Archive Ar(Path::ConvertProjectPath(Path));
			
			if (Ar.IsValid())
			{
				uint16 TypeByte;
				Ar >> TypeByte;
				EAssetType Type = static_cast<EAssetType>(TypeByte);
			
				if ( Type == T::GetAssetTypeStatic() )
				{
					asset = new T(Path);
					m_AssetRegistery[Path] = asset;

					asset->AddRef();
					return static_cast<T*>(asset);
				}
			}
		}

		return nullptr;
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