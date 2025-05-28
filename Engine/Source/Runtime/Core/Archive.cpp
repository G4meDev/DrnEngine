#include "DrnPCH.h"
#include "Archive.h"

LOG_DEFINE_CATEGORY( LogArchive, "Archive" );

#define ARCHIVE_VERSION ((uint8)2)

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
				m_ValidArchive = false;
				return;
			}

			*this >> m_ArchiveVersion;
			m_ValidArchive = true;
		}

		else
		{
			File = std::fstream( InFilePath, std::ios::out | std::ios::binary );
			if (!File)
			{
				LOG( LogArchive, Error, "failed to open file for writing. " );
				m_ValidArchive = false;
				return;
			}

			*this << ARCHIVE_VERSION;
			m_ValidArchive = true;
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
		File.write( (char*)(&Value), 1);
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
		File.write( (char*)( &Value ), sizeof(float));
		return *this;
	}

	Archive& Archive::operator<<( Guid Value )
	{
		*this << Value.A << Value.B << Value.C << Value.D;
		return *this;
	}

	Archive& Archive::operator<<( const std::string& Value )
	{
		uint32 size = Value.size();
		*this << size;
		File << Value;

		return *this;
	}

	Archive& Archive::operator<<( const std::vector<char>& Value )
	{
		uint64 size = (uint64)Value.size();
		*this << size;
		File.write(Value.data(), size);

		return *this;
	}

	Archive& Archive::operator<<( const Vector& Value )
	{
		*this << Value.GetX();
		*this << Value.GetY();
		*this << Value.GetZ();

		return *this;
	}

	Archive& Archive::operator<<( const Vector4& Value )
	{
		*this << Value.GetX();
		*this << Value.GetY();
		*this << Value.GetZ();
		*this << Value.GetW();

		return *this;
	}

	Archive& Archive::operator<<( const Quat& Value )
	{
		*this << Value.GetX();
		*this << Value.GetY();
		*this << Value.GetZ();
		*this << Value.GetW();

		return *this;
	}

	Archive& Archive::operator<<( const Transform& Value )
	{
		*this << Value.Location;
		*this << Value.Rotation;
		*this << Value.Scale;

		return *this;
	}

	Archive& Archive::operator<<( bool Value )
	{
		File.write( (char*)( &Value ), 1);
		return *this;
	}

	Archive& Archive::operator<<( ID3DBlob* Value )
	{
		if (Value)
		{
			uint32 Size = Value->GetBufferSize();
			*this << Size;
			File.write( (char*)(Value->GetBufferPointer()), Size );
		}

		else
		{
			*this << (uint32)(0);
		}

		return *this;
	}

	Archive& Archive::operator>>( uint8& Value )
	{
		File.read( (char*)(&Value), 1 );
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
		File.read( (char*)(&Value), sizeof(float) );
		
		return *this;
	}

	Archive& Archive::operator>>( Guid& Value )
	{
		*this >> Value.A >> Value.B >> Value.C >> Value.D;
		return *this;
	}

	Archive& Archive::operator>>( std::string& Value )
	{
		uint32 size;
		*this >> size;

		std::vector<char> buffer(size);
		File.read(buffer.data(), size);

		Value = std::string(buffer.begin(), buffer.end());

		return *this;
	}

	Archive& Archive::operator>>( std::vector<char>& Value )
	{
		uint64 size;
		*this >> size;
		Value.resize(size);

		File.read( Value.data(), size );
		return *this;
	}

	Archive& Archive::operator>>( Vector& Value )
	{
		float X, Y, Z;
		*this >> X >> Y >> Z;

		Value = Vector(X, Y, Z);
		return *this;
	}

	Archive& Archive::operator>>( Vector4& Value )
	{
		float X, Y, Z, W;
		*this >> X >> Y >> Z >> W;

		Value = Vector4(X, Y, Z, W);
		return *this;
	}

	Archive& Archive::operator>>( Quat& Value )
	{
		float X, Y, Z, W;
		*this >> X >> Y >> Z >> W;

		Value = Quat(X, Y, Z, W);
		return *this;
	}

	Archive& Archive::operator>>( Transform& Value )
	{
		Vector Location;
		Quat Rotation;
		Vector Scale;

		*this >> Location >> Rotation >> Scale;
		Value = Transform(Location, Rotation, Scale);

		return *this;
	}

	Archive& Archive::operator>>( bool& Value )
	{
		File.read( (char*)(&Value), 1 );
		return *this;
	}

	Archive& Archive::operator>>( ID3DBlob*& Value )
	{
		Value = nullptr;

		uint32 Size;
		*this >> Size;

		if (Size > 0)
		{
			D3DCreateBlob(Size, &Value);
			File.read( (char*)(Value->GetBufferPointer()), Size );
		}

		return *this;
	}

}