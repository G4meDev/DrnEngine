#pragma once

#include "ForwardTypes.h"
#include "Runtime/Core/Archive.h"
#include "Runtime/Core/FileArchive.h"

namespace Drn
{
	class BufferArchive : public Archive
	{
	public:

		BufferArchive( uint64 Size, bool InIsLoading = true );
		virtual ~BufferArchive();

		//BufferArchive& operator=( BufferArchive& Other ) = delete;
		//BufferArchive& operator=( BufferArchive&& Other ) noexcept;

		void Compress();
		void Decompress();

		virtual BufferArchive& operator<<(bool Value) override;
		virtual BufferArchive& operator<<(uint8 Value) override;
		virtual BufferArchive& operator<<(uint16 Value) override;
		virtual BufferArchive& operator<<(uint32 Value) override;
		virtual BufferArchive& operator<<(uint64 Value) override;

		virtual BufferArchive& operator<<( int32 Value ) override;

		virtual BufferArchive& operator<<(float Value) override;
		virtual BufferArchive& operator<<(Float16 Value) override;
		virtual BufferArchive& operator<<(Guid Value) override;
		virtual BufferArchive& operator<<(const Vector& Value) override;
		virtual BufferArchive& operator<<(const Vector4& Value) override;
		virtual BufferArchive& operator<<(const Vector2& Value) override;
		virtual BufferArchive& operator<<(const Vector2Half& Value) override;
		virtual BufferArchive& operator<<(const Color& Value) override;
		virtual BufferArchive& operator<<(const Quat& Value) override;
		virtual BufferArchive& operator<<(const Transform& Value) override;
		virtual BufferArchive& operator<<(const std::string& Value) override;

		template<typename T>
		BufferArchive& WriteVector(const std::vector<T>& Value);

		virtual BufferArchive& operator<<(const std::vector<char>& Value) override;
		virtual BufferArchive& operator<<(const std::vector<uint8>& Value) override;
		virtual BufferArchive& operator<<(const std::vector<Vector>& Value) override;
		virtual BufferArchive& operator<<(const std::vector<Vector4>& Value) override;
		virtual BufferArchive& operator<<(const std::vector<Vector2>& Value) override;
		virtual BufferArchive& operator<<(const std::vector<Vector2Half>& Value) override;
		virtual BufferArchive& operator<<(const std::vector<uint32>& Value) override;
		virtual BufferArchive& operator<<(const std::vector<uint16>& Value) override;
		virtual BufferArchive& operator<<(const std::vector<Color>& Value) override;
		virtual BufferArchive& operator<<(ID3DBlob* Value) override;
		virtual BufferArchive& operator<<(const BufferArchive& Value) override;


		virtual BufferArchive& operator>>(bool& Value) override;
		virtual BufferArchive& operator>>(uint8& Value) override;
		virtual BufferArchive& operator>>(uint16& Value) override;
		virtual BufferArchive& operator>>(uint32& Value) override;
		virtual BufferArchive& operator>>(uint64& Value) override;

		virtual BufferArchive& operator>>( int32& Value ) override;

		virtual BufferArchive& operator>>(float& Value) override;
		virtual BufferArchive& operator>>(Float16& Value) override;
		virtual BufferArchive& operator>>(Guid& Value) override;
		virtual BufferArchive& operator>>(Vector& Value) override;
		virtual BufferArchive& operator>>(Vector4& Value) override;
		virtual BufferArchive& operator>>(Vector2& Value) override;
		virtual BufferArchive& operator>>(Vector2Half& Value) override;
		virtual BufferArchive& operator>>(Color& Value) override;
		virtual BufferArchive& operator>>(Quat& Value) override;
		virtual BufferArchive& operator>>(Transform& Value) override;
		virtual BufferArchive& operator>>(std::string& Value) override;

		template<typename T>
		BufferArchive& ReadVector(std::vector<T>& Value);

		virtual BufferArchive& operator>>(std::vector<char>& Value) override;
		virtual BufferArchive& operator>>(std::vector<uint8>& Value) override;
		virtual BufferArchive& operator>>(std::vector<Vector>& Value) override;
		virtual BufferArchive& operator>>(std::vector<Vector4>& Value) override;
		virtual BufferArchive& operator>>(std::vector<Vector2>& Value) override;
		virtual BufferArchive& operator>>(std::vector<Vector2Half>& Value) override;
		virtual BufferArchive& operator>>(std::vector<uint32>& Value) override;
		virtual BufferArchive& operator>>(std::vector<uint16>& Value) override;
		virtual BufferArchive& operator>>(std::vector<Color>& Value) override;
		virtual BufferArchive& operator>>(ID3DBlob*& Value) override;
		virtual BufferArchive& operator>>(BufferArchive& Value) override;

		void CheckForAvaliableSpaceAndExpandConditional(uint64 Size);

		inline uint64 GetPointerIndex() const { return m_HeapPointer - m_HeapStart; };
		inline uint8* GetBufferPointer() const { return m_HeapStart; };

		inline void ResetPointer() { m_HeapPointer = m_HeapStart; }

	protected:

		uint64 m_Size;
		uint8* m_HeapStart;
		uint8* m_HeapPointer;

		friend class FileArchive;

	private:
	};
}