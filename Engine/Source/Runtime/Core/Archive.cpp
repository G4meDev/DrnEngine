#include "DrnPCH.h"
#include "Archive.h"

LOG_DEFINE_CATEGORY( LogArchive, "Archive" );

#define ARCHIVE_VERSION ((uint8)1)

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

			File >> m_ArchiveVersion;
		}

		else
		{
			File = std::fstream( InFilePath, std::ios::out | std::ios::binary );
			if (!File)
			{
				LOG( LogArchive, Error, "failed to open file for writing. " );
				return;
			}

			File << ARCHIVE_VERSION;
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
		File.write( (char*)(&Value), 2);
		return *this;
	}

	Archive& Archive::operator<<( uint32 Value )
	{
		File.write( (char*)( &Value ), 4);
		return *this;
	}

	Archive& Archive::operator<<( uint64 Value )
	{
		File.write( (char*)( &Value ), 8);
		return *this;
	}

	Archive& Archive::operator<<( float Value )
	{
		File << Value;
		return *this;
	}
	
	Archive& Archive::operator<<( const std::string& Value )
	{
		uint32 size = Value.size();
		File << size << Value;

		return *this;
	}

	Archive& Archive::operator<<( const std::vector<char>& Value )
	{
		uint64 size = (uint64)Value.size();
		File << size;
		File.write(Value.data(), size);

		return *this;
	}

	Archive& Archive::operator>>( uint8& Value )
	{
		File >> Value;
		return *this;
	}

	Archive& Archive::operator>>( uint16& Value )
	{
		File.read( (char*)(&Value), 2 );
		return *this;
	}

	Archive& Archive::operator>>( uint32& Value )
	{
		File.read( (char*)(&Value), 4 );
		return *this;
	}

	Archive& Archive::operator>>( uint64& Value )
	{
		File.read( (char*)(&Value), 8 );
		return *this;
	}

	Archive& Archive::operator>>( float& Value )
	{
		File >> Value;
		return *this;
	}

	Archive& Archive::operator>>( std::string& Value )
	{
		uint32 size;
		File >> size;

		std::vector<char> buffer(size);
		File.read(buffer.data(), size);

		Value = std::string(buffer.begin(), buffer.end());

		return *this;
	}

	Archive& Archive::operator>>( std::vector<char>& Value )
	{
		uint64 size;
		File >> size;
		Value.resize(size);

		File.read( Value.data(), size );
		return *this;
	}
}