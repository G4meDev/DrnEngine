#pragma once

#include "ForwardTypes.h"
#include "Runtime/Core/Archive.h"
#include "Runtime/Core/BufferArchive.h"

namespace Drn
{
	class FileArchive : public Archive
	{
	public:

		FileArchive( const std::string& InFilePath, bool InIsLoading = true );
		virtual ~FileArchive();

		inline std::string GetFilePath() { return m_FilePath; };

		virtual FileArchive& operator<<(bool Value) override;
		virtual FileArchive& operator<<(uint8 Value) override;
		virtual FileArchive& operator<<(uint16 Value) override;
		virtual FileArchive& operator<<(uint32 Value) override;
		virtual FileArchive& operator<<(uint64 Value) override;

		virtual FileArchive& operator<<( int32 Value ) override;

		virtual FileArchive& operator<<(float Value) override;
		virtual FileArchive& operator<<(Guid Value) override;
		virtual FileArchive& operator<<(const Vector& Value) override;
		virtual FileArchive& operator<<(const Vector4& Value) override;
		virtual FileArchive& operator<<(const Quat& Value) override;
		virtual FileArchive& operator<<(const Transform& Value) override;
		virtual FileArchive& operator<<(const std::string& Value) override;
		virtual FileArchive& operator<<(const std::vector<char>& Value) override;
		virtual FileArchive& operator<<(ID3DBlob* Value) override;
		virtual FileArchive& operator<<(const BufferArchive& Value) override;

		virtual FileArchive& operator<<( const PxMemoryStream& Value ) override;

		virtual FileArchive& operator>>(bool& Value) override;
		virtual FileArchive& operator>>(uint8& Value) override;
		virtual FileArchive& operator>>(uint16& Value) override;
		virtual FileArchive& operator>>(uint32& Value) override;
		virtual FileArchive& operator>>(uint64& Value) override;

		virtual FileArchive& operator>>( int32& Value ) override;

		virtual FileArchive& operator>>(float& Value) override;
		virtual FileArchive& operator>>(Guid& Value) override;
		virtual FileArchive& operator>>(Vector& Value) override;
		virtual FileArchive& operator>>(Vector4& Value) override;
		virtual FileArchive& operator>>(Quat& Value) override;
		virtual FileArchive& operator>>(Transform& Value) override;
		virtual FileArchive& operator>>(std::string& Value) override;
		virtual FileArchive& operator>>(std::vector<char>& Value) override;
		virtual FileArchive& operator>>(ID3DBlob*& Value) override;
		virtual FileArchive& operator>>(BufferArchive& Value) override;

		virtual FileArchive& operator>>( PxMemoryStream& Value ) override;

		void ReadWholeBuffer(std::vector<uint8>& Data);

	protected:

		std::string m_FilePath;
		std::fstream File;

		friend class BufferArchive;

	private:
	};
}