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
		static void GetFilesInDirectory(const std::string& Path, std::unique_ptr<SystemFileNode>& RootNode);

	private:
		static SystemFileNode* GetFilesInDirectory_Intern(const std::string& Path);
	};
}