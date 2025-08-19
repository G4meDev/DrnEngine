#pragma once

#include "ForwardTypes.h"
#include <fstream>

#include "Runtime/Math/Vector.h"
#include "Runtime/Math/Vector4.h"
#include "Runtime/Math/Rotator.h"
#include "Runtime/Math/Matrix.h"
#include "Runtime/Math/Transform.h"

#include "Runtime/Core/Guid.h"

#include <d3dcommon.h>

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

		Archive& operator<<(bool Value);
		Archive& operator<<(uint8 Value);
		Archive& operator<<(uint16 Value);
		Archive& operator<<(uint32 Value);
		Archive& operator<<(uint64 Value);

		Archive& operator<<( int32 Value );

		Archive& operator<<(float Value);
		Archive& operator<<(Guid Value);
		Archive& operator<<(const Vector& Value);
		Archive& operator<<(const Vector4& Value);
		Archive& operator<<(const Quat& Value);
		Archive& operator<<(const Transform& Value);
		Archive& operator<<(const std::string& Value);
		Archive& operator<<(const std::vector<char>& Value);
		Archive& operator<<(ID3DBlob* Value);


		Archive& operator>>(bool& Value);
		Archive& operator>>(uint8& Value);
		Archive& operator>>(uint16& Value);
		Archive& operator>>(uint32& Value);
		Archive& operator>>(uint64& Value);

		Archive& operator>>( int32& Value );

		Archive& operator>>(float& Value);
		Archive& operator>>(Guid& Value);
		Archive& operator>>(Vector& Value);
		Archive& operator>>(Vector4& Value);
		Archive& operator>>(Quat& Value);
		Archive& operator>>(Transform& Value);
		Archive& operator>>(std::string& Value);
		Archive& operator>>(std::vector<char>& Value);
		Archive& operator>>(ID3DBlob*& Value);

		inline bool IsValid() const { return m_ValidArchive; }
		inline uint8 GetVersion() const { return m_ArchiveVersion; };

		void ReadWholeBuffer(std::vector<uint8>& Data);

	protected:

		std::string m_FilePath;
		bool m_IsLoading;

		uint8 m_ArchiveVersion;

		std::fstream File;
		bool m_ValidArchive = false;

	private:
	};
}