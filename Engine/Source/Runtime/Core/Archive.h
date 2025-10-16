#pragma once

#include "ForwardTypes.h"
#include <fstream>

#include "Runtime/Math/Vector.h"
#include "Runtime/Math/Vector4.h"
#include "Runtime/Math/Vector2.h"
#include "Runtime/Math/Rotator.h"
#include "Runtime/Math/Matrix.h"
#include "Runtime/Math/Transform.h"

#include "Runtime/Core/Guid.h"
#include "Runtime/Physic/BodySetup.h"

#include <d3dcommon.h>

LOG_DECLARE_CATEGORY( LogArchive );
#define ARCHIVE_VERSION ( (uint8)1 )

namespace Drn
{
	class Archive
	{
	public:
		Archive(bool InIsLoading = true);
		virtual ~Archive();

		inline bool IsLoading() { return m_IsLoading; };
		inline uint8 GetVersion() const { return m_ArchiveVersion; };
		inline bool IsValid() const { return m_ValidArchive; }

		virtual Archive& operator<<(bool Value) = 0;
		virtual Archive& operator<<(uint8 Value) = 0;
		virtual Archive& operator<<(uint16 Value) = 0;
		virtual Archive& operator<<(uint32 Value) = 0;
		virtual Archive& operator<<(uint64 Value) = 0;

		virtual Archive& operator<<( int32 Value ) = 0;

		virtual Archive& operator<<(float Value) = 0;
		virtual Archive& operator<<(Guid Value) = 0;
		virtual Archive& operator<<(const Vector& Value) = 0;
		virtual Archive& operator<<(const Vector4& Value) = 0;
		virtual Archive& operator<<(const Vector2& Value) = 0;
		virtual Archive& operator<<(const Quat& Value) = 0;
		virtual Archive& operator<<(const Transform& Value) = 0;
		virtual Archive& operator<<(const std::string& Value) = 0;
		virtual Archive& operator<<(const std::vector<char>& Value) = 0;
		virtual Archive& operator<<(const std::vector<uint8>& Value) = 0;
		virtual Archive& operator<<(const std::vector<Vector>& Value) = 0;
		virtual Archive& operator<<(const std::vector<Vector4>& Value) = 0;
		virtual Archive& operator<<(const std::vector<Vector2>& Value) = 0;
		virtual Archive& operator<<(const std::vector<uint32>& Value) = 0;
		virtual Archive& operator<<(ID3DBlob* Value) = 0;
		virtual Archive& operator<<(const BufferArchive& Value) = 0;

		virtual Archive& operator>>(bool& Value) = 0;
		virtual Archive& operator>>(uint8& Value) = 0;
		virtual Archive& operator>>(uint16& Value) = 0;
		virtual Archive& operator>>(uint32& Value) = 0;
		virtual Archive& operator>>(uint64& Value) = 0;

		virtual Archive& operator>>( int32& Value ) = 0;

		virtual Archive& operator>>(float& Value) = 0;
		virtual Archive& operator>>(Guid& Value) = 0;
		virtual Archive& operator>>(Vector& Value) = 0;
		virtual Archive& operator>>(Vector4& Value) = 0;
		virtual Archive& operator>>(Vector2& Value) = 0;
		virtual Archive& operator>>(Quat& Value) = 0;
		virtual Archive& operator>>(Transform& Value) = 0;
		virtual Archive& operator>>(std::string& Value) = 0;
		virtual Archive& operator>>(std::vector<char>& Value) = 0;
		virtual Archive& operator>>(std::vector<uint8>& Value) = 0;
		virtual Archive& operator>>(std::vector<Vector>& Value) = 0;
		virtual Archive& operator>>(std::vector<Vector4>& Value) = 0;
		virtual Archive& operator>>(std::vector<Vector2>& Value) = 0;
		virtual Archive& operator>>(std::vector<uint32>& Value) = 0;
		virtual Archive& operator>>(ID3DBlob*& Value) = 0;
		virtual Archive& operator>>(BufferArchive& Value) = 0;

	protected:
		bool m_IsLoading;
		uint8 m_ArchiveVersion;
		bool m_ValidArchive = false;
	};


}