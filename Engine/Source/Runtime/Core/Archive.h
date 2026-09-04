#pragma once

#include "ForwardTypes.h"
#include <fstream>

#include "Runtime/Math/Vector.h"
#include "Runtime/Math/Vector4.h"
#include "Runtime/Math/Vector2.h"
#include "Runtime/Math/Rotator.h"
#include "Runtime/Math/Matrix.h"
#include "Runtime/Math/Transform.h"
#include "Runtime/Math/BoxSphereBounds.h"

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

		inline void SetLoading( bool bInLoading ) { m_IsLoading = bInLoading; }

		virtual uint64 Tell() = 0;
		virtual void Seek(uint64 InPos) = 0;
		//virtual uint64 TotalSize() const = 0;

		virtual Archive& operator<<(bool Value) = 0;
		virtual Archive& operator<<(uint8 Value) = 0;
		virtual Archive& operator<<(uint16 Value) = 0;
		virtual Archive& operator<<(uint32 Value) = 0;
		virtual Archive& operator<<(uint64 Value) = 0;

		virtual Archive& operator<<( int32 Value ) = 0;

		virtual Archive& operator<<(float Value) = 0;
		virtual Archive& operator<<(Float16 Value) = 0;
		virtual Archive& operator<<(Guid Value) = 0;
		virtual Archive& operator<<(const Vector& Value) = 0;
		virtual Archive& operator<<(const Vector4& Value) = 0;
		virtual Archive& operator<<(const Vector2& Value) = 0;
		virtual Archive& operator<<(const Vector2Half& Value) = 0;
		virtual Archive& operator<<(const Color& Value) = 0;
		virtual Archive& operator<<(const Quat& Value) = 0;
		virtual Archive& operator<<(const Matrix& Value);
		virtual Archive& operator<<(const Transform& Value) = 0;
		virtual Archive& operator<<(const std::string& Value) = 0;
		virtual Archive& operator<<(const std::vector<char>& Value) = 0;
		virtual Archive& operator<<(const std::vector<uint8>& Value) = 0;
		virtual Archive& operator<<(const std::vector<Vector>& Value) = 0;
		virtual Archive& operator<<(const std::vector<Vector4>& Value) = 0;
		virtual Archive& operator<<(const std::vector<Vector2>& Value) = 0;
		virtual Archive& operator<<(const std::vector<Vector2Half>& Value) = 0;
		virtual Archive& operator<<(const std::vector<uint32>& Value) = 0;
		virtual Archive& operator<<(const std::vector<uint16>& Value) = 0;
		virtual Archive& operator<<(const std::vector<Color>& Value) = 0;
		virtual Archive& operator<<(ID3DBlob* Value) = 0;
		virtual Archive& operator<<(const BufferArchive& Value) = 0;

		virtual Archive& operator>>(bool& Value) = 0;
		virtual Archive& operator>>(uint8& Value) = 0;
		virtual Archive& operator>>(uint16& Value) = 0;
		virtual Archive& operator>>(uint32& Value) = 0;
		virtual Archive& operator>>(uint64& Value) = 0;

		virtual Archive& operator>>( int32& Value ) = 0;

		virtual Archive& operator>>(float& Value) = 0;
		virtual Archive& operator>>(Float16& Value) = 0;
		virtual Archive& operator>>(Guid& Value) = 0;
		virtual Archive& operator>>(Vector& Value) = 0;
		virtual Archive& operator>>(Vector4& Value) = 0;
		virtual Archive& operator>>(Vector2& Value) = 0;
		virtual Archive& operator>>(Vector2Half& Value) = 0;
		virtual Archive& operator>>(Color& Value) = 0;
		virtual Archive& operator>>(Quat& Value) = 0;
		virtual Archive& operator>>(Matrix& Value);
		virtual Archive& operator>>(Transform& Value) = 0;
		virtual Archive& operator>>(std::string& Value) = 0;
		virtual Archive& operator>>(std::vector<char>& Value) = 0;
		virtual Archive& operator>>(std::vector<uint8>& Value) = 0;
		virtual Archive& operator>>(std::vector<Vector>& Value) = 0;
		virtual Archive& operator>>(std::vector<Vector4>& Value) = 0;
		virtual Archive& operator>>(std::vector<Vector2>& Value) = 0;
		virtual Archive& operator>>(std::vector<Vector2Half>& Value) = 0;
		virtual Archive& operator>>(std::vector<uint32>& Value) = 0;
		virtual Archive& operator>>(std::vector<uint16>& Value) = 0;
		virtual Archive& operator>>(std::vector<Color>& Value) = 0;
		virtual Archive& operator>>(ID3DBlob*& Value) = 0;
		virtual Archive& operator>>(BufferArchive& Value) = 0;

// ---------------------------------------------------------------------------

		Archive& operator<<(const BoxSphereBounds& Value);
		Archive& operator>>(BoxSphereBounds& Value);

		template<typename S, typename T>
		Archive& operator<<(const std::vector<T>& Value)
		{
			static_assert(!std::numeric_limits<S>::is_signed);

			constexpr uint64 ContaierSizeMax = std::numeric_limits<S>::max();

			const uint64 Size = Value.size();
			const bool bSizeOverflow = Size > ContaierSizeMax;
			drn_check(!bSizeOverflow);

			S ClampedSize = bSizeOverflow ? ContaierSizeMax : Size;
			*this << ClampedSize;
			for (S i = 0; i < ClampedSize; i++)
			{
				*this << Value[i];
			}
		
			return *this;
		}

		template<typename S, typename T>
		Archive& operator>>(std::vector<T>& Value)
		{
			static_assert(!std::numeric_limits<S>::is_signed);

			S Size;
			*this >> Size;
			Value.resize(Size);
		
			for (S i = 0; i < Size; i++)
			{
				*this >> Value[i];
			}
		
			return *this;
		}

		template<typename S, typename T>
		void Serialize(std::vector<T>& Value)
		{
			static_assert(!std::numeric_limits<S>::is_signed);

			if (IsLoading())
			{
				S Size;
				*this >> Size;
				Value.resize(Size);
		
				for (S i = 0; i < Size; i++)
				{
					Value[i].Serialize(*this);
				}
			}

			else
			{
				constexpr uint64 ContaierSizeMax = std::numeric_limits<S>::max();

				const uint64 Size = Value.size();
				const bool bSizeOverflow = Size > ContaierSizeMax;
				drn_check(!bSizeOverflow);

				S ClampedSize = bSizeOverflow ? ContaierSizeMax : Size;
				*this << ClampedSize;
				for (S i = 0; i < ClampedSize; i++)
				{
					Value[i].Serialize(*this);
				}
			}
		}

	protected:
		bool m_IsLoading;
		uint8 m_ArchiveVersion;
		bool m_ValidArchive = false;
	};


}