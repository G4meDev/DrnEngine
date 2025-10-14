#include "DrnPCH.h"
#include "BufferArchive.h"

#include <lz4.h>

namespace Drn
{
	BufferArchive::BufferArchive( uint64 Size, bool InIsLoading)
		: Archive(InIsLoading)
		, m_HeapStart(nullptr)
		, m_HeapPointer(nullptr)
		, m_Size(0)
	{
		if (Size > 0)
		{
			m_Size = Size;
			m_HeapStart = m_HeapPointer = new uint8[Size];

			if (InIsLoading) { }
			else { }

			m_ValidArchive = true;
		}
	}

	BufferArchive::~BufferArchive()
	{
		if (m_HeapStart)
			delete[] m_HeapStart;
	}

	void BufferArchive::Compress()
	{
		const uint32 SourceSize = GetPointerIndex();
		int32 Bound = LZ4_compressBound(SourceSize);

		uint8* CompressedBuffer = new uint8[Bound];
		int32 CompressionSize = LZ4_compress_default((char*)m_HeapStart, (char*)CompressedBuffer, SourceSize, Bound);

		delete[] m_HeapStart;
		m_Size = CompressionSize + 4;
		m_HeapStart = m_HeapPointer = new uint8[m_Size];

		*this << SourceSize;
		memcpy(m_HeapPointer, CompressedBuffer, CompressionSize);
		m_HeapPointer = m_HeapStart + m_Size;
		delete[] CompressedBuffer;
	}

	void BufferArchive::Decompress()
	{
		m_HeapPointer = m_HeapStart;
		int32 DecompressedSize;
		*this >> DecompressedSize;

		uint8* NewBuffer = new uint8[DecompressedSize];
		LZ4_decompress_fast((char*)m_HeapPointer, (char*)NewBuffer, DecompressedSize);

		delete[] m_HeapStart;
		m_HeapStart = m_HeapPointer = NewBuffer;
		m_Size = DecompressedSize;
	}

	// BufferArchive& BufferArchive::operator=( BufferArchive&& Other ) noexcept
	//{
	//	if (m_HeapStart)
	//		delete[] m_HeapStart;
	//
	//	m_HeapStart = Other.m_HeapStart;
	//	m_HeapPointer = Other.m_HeapPointer;
	//	m_Size = Other.m_Size;
	//	m_ValidArchive = Other.m_ValidArchive;
	//	m_ArchiveVersion = Other.m_ArchiveVersion;
	//
	//	Other.m_HeapStart = nullptr;
	//}

	BufferArchive& BufferArchive::operator<<( uint8 Value )
	{
		const uint32 Size = 1;
		CheckForAvaliableSpaceAndExpandConditional(Size);
		memcpy(m_HeapPointer, &Value, Size);
		m_HeapPointer += Size;
		return *this;
	}

	BufferArchive& BufferArchive::operator<<( uint16 Value )
	{
		const uint32 Size = 2;
		CheckForAvaliableSpaceAndExpandConditional(Size);
		memcpy(m_HeapPointer, &Value, Size);
		m_HeapPointer += Size;
		return *this;
	}

	BufferArchive& BufferArchive::operator<<( uint32 Value )
	{
		const uint32 Size = 4;
		CheckForAvaliableSpaceAndExpandConditional(Size);
		memcpy(m_HeapPointer, &Value, Size);
		m_HeapPointer += Size;
		return *this;
	}

	BufferArchive& BufferArchive::operator<<( uint64 Value )
	{
		const uint32 Size = 8;
		CheckForAvaliableSpaceAndExpandConditional(Size);
		memcpy(m_HeapPointer, &Value, Size);
		m_HeapPointer += Size;
		return *this;
	}

	BufferArchive& BufferArchive::operator<<( int32 Value )
	{
		const uint32 Size = 4;
		CheckForAvaliableSpaceAndExpandConditional(Size);
		memcpy(m_HeapPointer, &Value, Size);
		m_HeapPointer += Size;
		return *this;
	}

	BufferArchive& BufferArchive::operator<<( float Value )
	{
		const uint32 Size = 4;
		CheckForAvaliableSpaceAndExpandConditional(Size);
		memcpy(m_HeapPointer, &Value, Size);
		m_HeapPointer += Size;
		return *this;
	}

	BufferArchive& BufferArchive::operator<<( Guid Value )
	{
		*this << Value.A << Value.B << Value.C << Value.D;
		return *this;
	}

	BufferArchive& BufferArchive::operator<<( const std::string& Value )
	{
		uint32 Size = Value.size();
		*this << Size;

		CheckForAvaliableSpaceAndExpandConditional(Size);
		memcpy(m_HeapPointer, Value.data(), Size);
		m_HeapPointer += Size;
		return *this;
	}

	BufferArchive& BufferArchive::operator<<( const std::vector<char>& Value )
	{
		uint64 Size = (uint64)Value.size();
		*this << Size;

		CheckForAvaliableSpaceAndExpandConditional(Size);
		memcpy(m_HeapPointer, Value.data(), Size);
		m_HeapPointer += Size;
		return *this;
	}

	BufferArchive& BufferArchive::operator<<( const std::vector<uint8>& Value )
	{
		uint64 Size = (uint64)Value.size();
		*this << Size;

		CheckForAvaliableSpaceAndExpandConditional(Size);
		memcpy(m_HeapPointer, Value.data(), Size);
		m_HeapPointer += Size;
		return *this;
	}

	BufferArchive& BufferArchive::operator<<( const std::vector<Vector>& Value )
	{
		uint64 Count = (uint64)Value.size();
		uint64 Size = Count * sizeof(Vector);
		*this << Count;

		CheckForAvaliableSpaceAndExpandConditional(Size);

		for ( uint64 i = 0; i < Count; i++)
		{
			*this << Value[i];
			m_HeapPointer += sizeof(Vector);
		}

		//memcpy(m_HeapPointer, Value.data(), Size);
		//m_HeapPointer += Size;
		return *this;
	}

	BufferArchive& BufferArchive::operator<<( const std::vector<Vector4>& Value )
	{
		uint64 Count = (uint64)Value.size();
		uint64 Size = Count * sizeof(Vector4);
		*this << Count;

		CheckForAvaliableSpaceAndExpandConditional(Size);
		memcpy(m_HeapPointer, Value.data(), Size);
		m_HeapPointer += Size;
		return *this;
	}

	BufferArchive& BufferArchive::operator<<( const std::vector<Vector2>& Value )
	{
		uint64 Count = (uint64)Value.size();
		uint64 Size = Count * sizeof(Vector2);
		*this << Count;

		CheckForAvaliableSpaceAndExpandConditional(Size);
		memcpy(m_HeapPointer, Value.data(), Size);
		m_HeapPointer += Size;
		return *this;
	}

	BufferArchive& BufferArchive::operator<<( const std::vector<uint32>& Value )
	{
		uint64 Count = (uint64)Value.size();
		uint64 Size = Count * sizeof(uint32);
		*this << Count;

		CheckForAvaliableSpaceAndExpandConditional(Size);
		memcpy(m_HeapPointer, Value.data(), Size);
		m_HeapPointer += Size;
		return *this;
	}

	BufferArchive& BufferArchive::operator<<( const Vector& Value )
	{
		*this << Value.GetX();
		*this << Value.GetY();
		*this << Value.GetZ();

		return *this;
	}

	BufferArchive& BufferArchive::operator<<( const Vector4& Value )
	{
		*this << Value.GetX();
		*this << Value.GetY();
		*this << Value.GetZ();
		*this << Value.GetW();

		return *this;
	}

	BufferArchive& BufferArchive::operator<<( const Vector2& Value )
	{
		*this << Value.GetX();
		*this << Value.GetY();

		return *this;
	}

	BufferArchive& BufferArchive::operator<<( const Quat& Value )
	{
		*this << Value.GetX();
		*this << Value.GetY();
		*this << Value.GetZ();
		*this << Value.GetW();

		return *this;
	}

	BufferArchive& BufferArchive::operator<<( const Transform& Value )
	{
		*this << Value.Location;
		*this << Value.Rotation;
		*this << Value.Scale;

		return *this;
	}

	BufferArchive& BufferArchive::operator<<( bool Value )
	{
		const uint32 Size = 1;
		CheckForAvaliableSpaceAndExpandConditional(Size);
		memcpy(m_HeapPointer, &Value, Size);
		m_HeapPointer += Size;
		return *this;
	}

	BufferArchive& BufferArchive::operator<<( ID3DBlob* Value )
	{
		if (Value)
		{
			uint32 Size = Value->GetBufferSize();
			*this << Size;

			CheckForAvaliableSpaceAndExpandConditional(Size);
			memcpy(m_HeapPointer, Value->GetBufferPointer(), Size);
			m_HeapPointer += Size;
		}

		else
		{
			*this << (uint32)(0);
		}

		return *this;
	}

	BufferArchive& BufferArchive::operator<<( const BufferArchive& Value )
	{
		

		return *this;
	}

	BufferArchive& BufferArchive::operator>>( uint8& Value )
	{
		const uint32 Size = 1;
		memcpy(&Value, m_HeapPointer, Size);
		m_HeapPointer += Size;
		return *this;
	}

	BufferArchive& BufferArchive::operator>>( uint16& Value )
	{
		const uint32 Size = 2;
		memcpy(&Value, m_HeapPointer, Size);
		m_HeapPointer += Size;
		return *this;
	}

	BufferArchive& BufferArchive::operator>>( uint32& Value )
	{
		const uint32 Size = 4;
		memcpy(&Value, m_HeapPointer, Size);
		m_HeapPointer += Size;
		return *this;
	}

	BufferArchive& BufferArchive::operator>>( uint64& Value )
	{
		const uint32 Size = 8;
		memcpy(&Value, m_HeapPointer, Size);
		m_HeapPointer += Size;
		return *this;
	}

	BufferArchive& BufferArchive::operator>>( int32& Value )
	{
		const uint32 Size = 4;
		memcpy(&Value, m_HeapPointer, Size);
		m_HeapPointer += Size;
		return *this;
	}

	BufferArchive& BufferArchive::operator>>( float& Value )
	{
		const uint32 Size = 4;
		memcpy(&Value, m_HeapPointer, Size);
		m_HeapPointer += Size;
		return *this;
	}

	BufferArchive& BufferArchive::operator>>( Guid& Value )
	{
		*this >> Value.A >> Value.B >> Value.C >> Value.D;
		return *this;
	}

	BufferArchive& BufferArchive::operator>>( std::string& Value )
	{
		uint32 Size;
		*this >> Size;

		std::vector<char> buffer(Size);
		memcpy(buffer.data(), m_HeapPointer, Size);
		Value = std::string(buffer.begin(), buffer.end());
		m_HeapPointer += Size;

		return *this;
	}

	BufferArchive& BufferArchive::operator>>( std::vector<char>& Value )
	{
		uint64 Size;
		*this >> Size;

		Value.resize(Size);
		memcpy(Value.data(), m_HeapPointer, Size);
		m_HeapPointer += Size;
		return *this;
	}

	BufferArchive& BufferArchive::operator>>( std::vector<uint8>& Value )
	{
		uint64 Size;
		*this >> Size;

		Value.resize(Size);
		memcpy(Value.data(), m_HeapPointer, Size);
		m_HeapPointer += Size;
		return *this;
	}

	BufferArchive& BufferArchive::operator>>( std::vector<Vector>& Value )
	{
		uint64 Count;
		*this >> Count;

		uint64 Size = Count * sizeof(Vector);

		Value.resize(Size);

		for ( uint64 i = 0; i < Count; i++)
		{
			*this >> Value[i];
			m_HeapPointer += sizeof(Vector);
		}

		//memcpy(Value.data(), m_HeapPointer, Size);
		//m_HeapPointer += Size;
		return *this;
	}

	BufferArchive& BufferArchive::operator>>( std::vector<Vector4>& Value )
	{
		uint64 Count;
		*this >> Count;

		uint64 Size = Count * sizeof(Vector4);

		Value.resize(Size);
		memcpy(Value.data(), m_HeapPointer, Size);
		m_HeapPointer += Size;
		return *this;
	}

	BufferArchive& BufferArchive::operator>>( std::vector<Vector2>& Value )
	{
		uint64 Count;
		*this >> Count;

		uint64 Size = Count * sizeof(Vector2);

		Value.resize(Size);
		memcpy(Value.data(), m_HeapPointer, Size);
		m_HeapPointer += Size;
		return *this;
	}

	BufferArchive& BufferArchive::operator>>( std::vector<uint32>& Value )
	{
		uint64 Count;
		*this >> Count;

		uint64 Size = Count * sizeof(uint32);

		Value.resize(Size);
		memcpy(Value.data(), m_HeapPointer, Size);
		m_HeapPointer += Size;
		return *this;
	}

	BufferArchive& BufferArchive::operator>>( Vector& Value )
	{
		float X, Y, Z;
		*this >> X >> Y >> Z;

		Value = Vector(X, Y, Z);
		return *this;
	}

	BufferArchive& BufferArchive::operator>>( Vector4& Value )
	{
		float X, Y, Z, W;
		*this >> X >> Y >> Z >> W;

		Value = Vector4(X, Y, Z, W);
		return *this;
	}

	BufferArchive& BufferArchive::operator>>( Vector2& Value )
	{
		float X, Y;
		*this >> X >> Y;

		Value = Vector2(X, Y);
		return *this;
	}

	BufferArchive& BufferArchive::operator>>( Quat& Value )
	{
		float X, Y, Z, W;
		*this >> X >> Y >> Z >> W;

		Value = Quat(X, Y, Z, W);
		return *this;
	}

	BufferArchive& BufferArchive::operator>>( Transform& Value )
	{
		Vector Location;
		Quat Rotation;
		Vector Scale;

		*this >> Location >> Rotation >> Scale;
		Value = Transform(Location, Rotation, Scale);

		return *this;
	}

	BufferArchive& BufferArchive::operator>>( bool& Value )
	{
		const uint32 Size = 1;
		memcpy(&Value, m_HeapPointer, Size);
		m_HeapPointer += Size;
		return *this;
	}

	BufferArchive& BufferArchive::operator>>( ID3DBlob*& Value )
	{
		Value = nullptr;

		uint32 Size;
		*this >> Size;

		if (Size > 0)
		{
			D3DCreateBlob(Size, &Value);
			memcpy(Value->GetBufferPointer(), m_HeapPointer, Size);			
			m_HeapPointer += Size;
		}

		return *this;
	}

	BufferArchive& BufferArchive::operator>>( BufferArchive& Value )
	{
		

		return *this;
	}

	void BufferArchive::CheckForAvaliableSpaceAndExpandConditional( uint64 Size )
	{
		const uint64 PointerIndex = GetPointerIndex();
		const uint64 ExpectedSize = PointerIndex + Size;

		if ( Size > 0 && ExpectedSize > m_Size )
		{
			int32 Exp = std::floor(std::log2(ExpectedSize));
			uint64 NewSize = std::pow(2, std::max(0, Exp) + 1);

			uint8* NewBuffer = new uint8[NewSize];
			memcpy(NewBuffer, m_HeapStart, m_Size);

			if (m_HeapPointer)
				delete[] m_HeapStart;

			m_HeapStart = NewBuffer;
			m_HeapPointer = m_HeapStart + PointerIndex;
			m_Size = NewSize;
		}
	}

}