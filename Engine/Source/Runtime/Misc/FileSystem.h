#pragma once

#include "ForwardTypes.h"

LOG_DECLARE_CATEGORY(LogFileSystem);

namespace Drn
{
	struct SystemFile
	{
	public:
		SystemFile(const std::string& InFullPath, bool InIsDirectory);

		std::string m_ShortPath;
		std::string m_FullPath;
		bool m_IsDirectory;
	};

	struct SystemFileNode
	{
	public:
		SystemFileNode(const std::string& InFullPath, bool InIsDirectory);
		~SystemFileNode();

		SystemFile File;
		std::vector<SystemFileNode*> Childs;

		bool ContainsDirectory();
		std::vector<SystemFileNode*> GetFiles();
		int GetNumberOfFiles();

		void AddChild(SystemFileNode* Child);
	};

	class FileSystem
	{
	public:
		static bool DirectoryExists(const std::string& Path);
		static std::string DirectoryFromFilePath(const std::string& Path);
		static bool CreateDirectory(const std::string& Path);
		static bool CreateDirectoryIfDoesntExist(const std::string& Path);
		static bool FileExists(const std::string& Path);
		static void GetFilesInDirectory(const std::string& Path, std::unique_ptr<SystemFileNode>& RootNode, const std::string& Filter = "*");

		static std::string ReadFileAsString( const std::string& Path );

		static void WriteStringToFile( const std::string& Path, const std::string& Str, bool ForceDirectory = true);

	private:
		static SystemFileNode* GetFilesInDirectory_Intern(const std::string& Path, const std::string& Filter);
	};
}