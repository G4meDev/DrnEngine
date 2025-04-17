#pragma once

#include <DirectXMath.h>

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
		Transform(const Matrix& InMatrix);

		Transform operator*(const Transform& Other) const;

		Transform GetRelativeTransform(const Transform& RelativeTo) const;

		inline Vector GetLocation() const { return Location; }
		inline Quat GetRotation() const { return Rotation; }
		inline Vector GetScale() const { return Scale; }

		inline void SetLocation( const Vector& InLocation ) { Location = InLocation; }
		inline void SetRotation( const Quat& InRotation) { Rotation = InRotation; }
		inline void SetScale( const Vector& InScale ) { Scale = InScale; }

		inline static Vector SubtractTranslations(const Transform& A, const Transform& B) { return A.GetLocation() - B.GetLocation(); }

		inline bool Equals( const Transform& Other ) { return Location.Equals(Other.Location) && Rotation.Equals(Other.Rotation) && Scale.Equals(Other.Scale); }

	private:
		Vector Location;
		Quat Rotation;
		Vector Scale;

		friend class Matrix;
		friend class Archive;
	};
}