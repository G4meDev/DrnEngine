#include "DrnPCH.h"
#include "FileArchive.h"

namespace Drn
{
		FileArchive::FileArchive( const std::string& InFilePath, bool InIsLoading )
		: Archive(InIsLoading)
		, m_FilePath(InFilePath)
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
		}
		m_ValidArchive = true;
	}

	FileArchive::~FileArchive()
	{
		if (File)
		{
			File.close();
		}
	}

	FileArchive& FileArchive::operator<<( uint8 Value )
	{
		File.write( (char*)(&Value), 1);
		return *this;
	}

	FileArchive& FileArchive::operator<<( uint16 Value )
	{
		File.write( (char*)(&Value), 2);
		return *this;
	}

	FileArchive& FileArchive::operator<<( uint32 Value )
	{
		File.write( (char*)( &Value ), 4);
		return *this;
	}

	FileArchive& FileArchive::operator<<( uint64 Value )
	{
		File.write( (char*)( &Value ), 8);
		return *this;
	}

	FileArchive& FileArchive::operator<<( int32 Value )
	{
		File.write( (char*)( &Value ), 4);
		return *this;
	}

	FileArchive& FileArchive::operator<<( float Value )
	{
		File.write( (char*)( &Value ), sizeof(float));
		return *this;
	}

	FileArchive& FileArchive::operator<<( Guid Value )
	{
		*this << Value.A << Value.B << Value.C << Value.D;
		return *this;
	}

	FileArchive& FileArchive::operator<<( const std::string& Value )
	{
		uint32 size = Value.size();
		*this << size;
		File << Value;

		return *this;
	}

	FileArchive& FileArchive::operator<<( const std::vector<char>& Value )
	{
		uint64 size = (uint64)Value.size();
		*this << size;
		File.write(Value.data(), size);

		return *this;
	}

	FileArchive& FileArchive::operator<<( const Vector& Value )
	{
		*this << Value.GetX();
		*this << Value.GetY();
		*this << Value.GetZ();

		return *this;
	}

	FileArchive& FileArchive::operator<<( const Vector4& Value )
	{
		*this << Value.GetX();
		*this << Value.GetY();
		*this << Value.GetZ();
		*this << Value.GetW();

		return *this;
	}

	FileArchive& FileArchive::operator<<( const Quat& Value )
	{
		*this << Value.GetX();
		*this << Value.GetY();
		*this << Value.GetZ();
		*this << Value.GetW();

		return *this;
	}

	FileArchive& FileArchive::operator<<( const Transform& Value )
	{
		*this << Value.Location;
		*this << Value.Rotation;
		*this << Value.Scale;

		return *this;
	}

	FileArchive& FileArchive::operator<<( bool Value )
	{
		File.write( (char*)( &Value ), 1);
		return *this;
	}

	FileArchive& FileArchive::operator<<( ID3DBlob* Value )
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

	FileArchive& FileArchive::operator<<( const BufferArchive& Value )
	{
		uint64 Size = Value.GetPointerIndex();
		*this << Size;
		File.write( (char*)(Value.GetBufferPointer()), Size );
		return *this;
	}

	FileArchive& FileArchive::operator>>( uint8& Value )
	{
		File.read( (char*)(&Value), 1 );
		return *this;
	}

	FileArchive& FileArchive::operator>>( uint16& Value )
	{
		File.read( (char*)(&Value), 2 );
		return *this;
	}

	FileArchive& FileArchive::operator>>( uint32& Value )
	{
		File.read( (char*)(&Value), 4 );
		return *this;
	}

	FileArchive& FileArchive::operator>>( uint64& Value )
	{
		File.read( (char*)(&Value), 8 );
		return *this;
	}

	FileArchive& FileArchive::operator>>( int32& Value )
	{
		File.read( (char*)(&Value), 4 );
		return *this;
	}

	FileArchive& FileArchive::operator>>( float& Value )
	{
		File.read( (char*)(&Value), sizeof(float) );
		
		return *this;
	}

	FileArchive& FileArchive::operator>>( Guid& Value )
	{
		*this >> Value.A >> Value.B >> Value.C >> Value.D;
		return *this;
	}

	FileArchive& FileArchive::operator>>( std::string& Value )
	{
		uint32 size;
		*this >> size;

		std::vector<char> buffer(size);
		File.read(buffer.data(), size);

		Value = std::string(buffer.begin(), buffer.end());

		return *this;
	}

	FileArchive& FileArchive::operator>>( std::vector<char>& Value )
	{
		uint64 size;
		*this >> size;
		Value.resize(size);

		File.read( Value.data(), size );
		return *this;
	}

	FileArchive& FileArchive::operator>>( Vector& Value )
	{
		float X, Y, Z;
		*this >> X >> Y >> Z;

		Value = Vector(X, Y, Z);
		return *this;
	}

	FileArchive& FileArchive::operator>>( Vector4& Value )
	{
		float X, Y, Z, W;
		*this >> X >> Y >> Z >> W;

		Value = Vector4(X, Y, Z, W);
		return *this;
	}

	FileArchive& FileArchive::operator>>( Quat& Value )
	{
		float X, Y, Z, W;
		*this >> X >> Y >> Z >> W;

		Value = Quat(X, Y, Z, W);
		return *this;
	}

	FileArchive& FileArchive::operator>>( Transform& Value )
	{
		Vector Location;
		Quat Rotation;
		Vector Scale;

		*this >> Location >> Rotation >> Scale;
		Value = Transform(Location, Rotation, Scale);

		return *this;
	}

	FileArchive& FileArchive::operator>>( bool& Value )
	{
		File.read( (char*)(&Value), 1 );
		return *this;
	}

	FileArchive& FileArchive::operator>>( ID3DBlob*& Value )
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

	FileArchive& FileArchive::operator>>( BufferArchive& Value )
	{
		uint64 Size;
		*this >> Size;

		Value.CheckForAvaliableSpaceAndExpandConditional( Size );
		File.read( (char*)(Value.GetBufferPointer()), Size );
		Value.m_ArchiveVersion = GetVersion();
		Value.m_ValidArchive = true;

		return *this;
	}

	void FileArchive::ReadWholeBuffer( std::vector<uint8>& Data )
	{
		File.seekg(0, std::ios::end);
		const uint64 Size = File.tellg();

		File.seekg(0, std::ios::beg);
		Data.resize(Size);
		File.read(reinterpret_cast<char*>(Data.data()), Size);
	}
}