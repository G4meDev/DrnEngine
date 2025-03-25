#pragma once

#include "ForwardTypes.h"
#include <fstream>

LOG_DECLARE_CATEGORY( LogArchive );

namespace Drn
{
	class Archive
	{
	public:

		Archive(const std::string& InFilePath, bool InIsLoading = true);
		~Archive();

		inline bool IsLoading() { return m_IsLoading; };
		inline std::string GetFilePath() { return m_FilePath; };

		Archive& operator<<(uint8 Value);
		Archive& operator<<(uint16 Value);
		Archive& operator<<(const std::string& Value);


		Archive& operator>>(uint8& Value);
		Archive& operator>>(uint16& Value);
		Archive& operator>>(std::string& Value);

	protected:

		std::string m_FilePath;
		bool m_IsLoading;

		std::fstream File;

	private:
	};
}