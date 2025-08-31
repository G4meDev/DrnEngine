#pragma once

#include <DirectXMath.h>
#include <string>
#include <sstream>

#include "Vector4.h"
#include "Vector.h"
#include "Quat.h"
#include "Matrix.h"

using namespace DirectX;

namespace Drn
{
	class Transform
	{
	public:

		Transform() : Transform(Vector::ZeroVector, Quat(), Vector::OneVector) {};
		inline Transform(const Vector& InLocation, const Quat& InRotation, const Vector& InScale) : Location(InLocation), Rotation(InRotation), Scale(InScale) {};
		inline Transform(const Vector& InLocation, const Quat& InRotation) : Location(InLocation), Rotation(InRotation), Scale(Vector::OneVector) {};
		Transform(const Matrix& InMatrix);

		Transform operator*(const Transform& Other) const;

		Transform GetRelativeTransform(const Transform& RelativeTo) const;
		Vector InverseTransformPosition( const Vector& InVector ) const;
		inline static Vector SubtractTranslations( const Transform& A, const Transform& B) { return A.Location - B.Location; }

		inline Vector GetLocation() const { return Location; }
		inline Quat GetRotation() const { return Rotation; }
		inline Vector GetScale() const { return Scale; }

		inline void SetLocation( const Vector& InLocation ) { Location = InLocation; }
		inline void SetRotation( const Quat& InRotation) { Rotation = InRotation; }
		inline void SetScale( const Vector& InScale ) { Scale = InScale; }

		Vector TransformVectorNoScale(const Vector& V) const;

		inline bool Equals( const Transform& Other ) { return Location.Equals(Other.Location) && Rotation.Equals(Other.Rotation) && Scale.Equals(Other.Scale); }

		inline Vector TransformPosition(const Vector& Pos) const
		{
			return XMVector3Transform(XMLoadFloat3(&Pos.m_Vector), Matrix(*this).Get());
		}

		Vector GetSafeScaleReciprocal( const Vector& InScale ) const;

		inline std::string ToString()
		{
			std::stringstream ss;
			ss << "(T: " << Location.ToString() << ", Q: " << Rotation.ToString() << ", S: " << Scale.ToString() << ")";

			return ss.str();
		}

		static Transform Identity;

	private:
		Vector Location;
		Quat Rotation;
		Vector Scale;

		friend class Matrix;
		friend class Archive;
		friend class FileArchive;
		friend class BufferArchive;
	};
}