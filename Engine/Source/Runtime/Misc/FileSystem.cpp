#include "DrnPCH.h"
#include "FileSystem.h"

LOG_DEFINE_CATEGORY( LogFileSystem, "FileSystem" );

namespace Drn
{
	SystemFile::SystemFile(const std::string& InFullPath, bool InIsDirectory)
		: m_FullPath(InFullPath)
		, m_IsDirectory(InIsDirectory)
	{	
		size_t LastSlash = m_FullPath.find_last_of("\\");
		if (LastSlash < 0)
		{
			m_ShortPath = m_FullPath;
		}
		else
		{
			m_ShortPath = m_FullPath.substr(LastSlash + 1, -1);
		}
	}

	SystemFileNode::SystemFileNode(const std::string& InFullPath, bool InIsDirectory)
		: File(InFullPath, InIsDirectory)
	{
		
	}

	SystemFileNode::~SystemFileNode()
	{
		for (SystemFileNode* Child : Childs)
		{
			delete Child;
			Child = nullptr;
		}
	}

	bool SystemFileNode::ContainsDirectory()
	{
		for (SystemFileNode* Child : Childs)
		{
			if (Child && Child->File.m_IsDirectory)
			{
				return true;
			}
		}

		return false;
	}

	std::vector<SystemFileNode*> SystemFileNode::GetFiles()
	{
		std::vector<SystemFileNode*> Result;
		Result.reserve(Childs.size());

		for (SystemFileNode* Child : Childs)
		{
			if (Child && !Child->File.m_IsDirectory)
			{
				Result.push_back(Child);
			}
		}

		return Result;
	}

	int SystemFileNode::GetNumberOfFiles()
	{
		return GetFiles().size();
	}

	void SystemFileNode::AddChild( SystemFileNode* Child )
	{
		Childs.push_back(Child);
	}

	bool FileSystem::DirectoryExists( const std::string& Path )
	{
		if (std::filesystem::exists(Path) && std::filesystem::is_directory(Path))
		{
			return true;
		}

		return false;
	}

	bool FileSystem::FileExists( const std::string& Path )
	{
		if (std::filesystem::exists(Path) && !std::filesystem::is_directory(Path))
		{
			return true;
		}

		return false;
	}

	void FileSystem::GetFilesInDirectory( const std::string& Path, std::unique_ptr<SystemFileNode>& RootNode )
	{
		RootNode.reset();
		RootNode = std::unique_ptr<SystemFileNode>(GetFilesInDirectory_Intern(Path));
	}

	SystemFileNode* FileSystem::GetFilesInDirectory_Intern( const std::string& Path )
	{
		if (DirectoryExists(Path))
		{
			SystemFileNode* Root = new SystemFileNode(Path, true);

			for (const auto& file : std::filesystem::directory_iterator(Path))
			{
				if (file.exists())
				{
					if (file.is_directory())
					{
						Root->AddChild(GetFilesInDirectory_Intern(file.path().string()));
					}
					else
					{
						
						SystemFileNode* FileNode = new SystemFileNode(file.path().string(), false);
						Root->AddChild(FileNode);
					}
				}
			}

			return Root;
		}
		
		LOG(LogFileSystem, Warning, "directory doest exist %s", Path.c_str());
		return nullptr;
	}
}