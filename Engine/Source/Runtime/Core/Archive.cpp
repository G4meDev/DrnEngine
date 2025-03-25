#include "DrnPCH.h"
#include "Archive.h"

LOG_DEFINE_CATEGORY( LogArchive, "Archive" );

namespace Drn
{
	Archive::Archive( const std::string& InFilePath, bool InIsLoading )
		: m_FilePath(InFilePath)
		, m_IsLoading(InIsLoading)
	{
		if (InIsLoading)
		{
			File = std::fstream( InFilePath, std::ios::in | std::ios::binary );
			if (!File)
			{
				LOG( LogArchive, Error, "failed to open file for reading. " );
				return;
			}
		}

		else
		{
			File = std::fstream( InFilePath, std::ios::out | std::ios::binary );
			if (!File)
			{
				LOG( LogArchive, Error, "failed to open file for writing. " );
				return;
			}
		}
	}

	Archive::~Archive()
	{
		if (File)
		{
			File.close();
		}
	}

	Archive& Archive::operator<<( uint8 Value )
	{
		File << Value;

		return *this;
	}

	Archive& Archive::operator<<( uint16 Value )
	{
		File << Value;

		return *this;
	}
	
	Archive& Archive::operator<<( const std::string& Value )
	{
		uint16 size = Value.size();
		File << size << Value;

		return *this;
	}

	Archive& Archive::operator>>( uint8& Value )
	{
		File >> Value;
		return *this;
	}

	Archive& Archive::operator>>( uint16& Value )
	{
		File >> Value;
		return *this;
	}

	Archive& Archive::operator>>( std::string& Value )
	{
		uint16 size;
		File >> size;

		std::vector<char> buffer(size);
		File.read(buffer.data(), size);

		Value = std::string(buffer.begin(), buffer.end());

		return *this;
	}
}