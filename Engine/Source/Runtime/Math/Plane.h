#pragma once

#include "Runtime/Math/Vector.h"

namespace Drn
{
	struct Plane
	{
	public:

		union
		{
			struct { float X, Y, Z; };
			Vector Normal;
		};
		float W;

		inline Plane()
			: Normal(Vector::ZeroVector)
			, W(0.0f)
		{}

		inline Plane(float InX, float InY, float InZ, float InW)
			: Normal(Vector(InX, InY, InZ))
			, W(InW)
		{}

		inline Plane(const Vector& InNormal, float InW)
			: Normal(InNormal)
			, W(InW)
		{}

		inline Plane(const Vector& InBase, const Vector& InNormal)
			: Normal(InNormal)
			, W(InBase | InNormal)
		{}

		inline Plane(const Vector& A, const Vector& B, const Vector& C)
			: Normal(((B - A) ^ (C - A)).GetSafeNormal())
		{
			W = A | (Normal);
		}

		inline bool IsValid() const
		{
			return !Normal.IsNearlyZero();
		}

		inline const Vector& GetNormal() const { return Normal; }

		inline Vector GetOrigin() const { return Normal * W; }

		inline float PlaneDot(const Vector& P) const
		{
			return (Normal | P) - W;
		}

		inline bool Normalize(float Tolerance = KINDA_SMALL_NUMBER)
		{
			const float SquareSum = Normal.X*Normal.X + Normal.Y*Normal.Y + Normal.Z*Normal.Z;
			if(SquareSum > Tolerance)
			{
				const float Scale = 1.0f / std::sqrt(SquareSum);
				Normal.X *= Scale; Normal.Y *= Scale; Normal.Z *= Scale; W *= Scale;
				return true;
			}
			return false;
		}

		inline Plane Flip() const
		{
			return Plane(-Normal.X, -Normal.Y, -Normal.Z, W);
		}

		inline Vector MirrorPoint(const Vector& P) const
		{
			return P - GetNormal() * (2.f * PlaneDot(P) );
		}

		inline Vector ProjectPoint(const Vector& P) const
		{
			return P - GetNormal() * PlaneDot(P);
		}

		inline Vector RayIntersection( const Vector& RayOrigin, const Vector& RayDirection ) const
		{
			const Vector& PlaneNormal = GetNormal();
			const Vector PlaneOrigin = GetOrigin();
		
			const float Distance = Vector::DotProduct( ( PlaneOrigin - RayOrigin ), PlaneNormal ) / Vector::DotProduct( RayDirection, PlaneNormal );
			return RayOrigin + RayDirection * Distance;
		}

		inline Vector LineIntersection(const Vector &Point1, const Vector &Point2) const
		{
			return Point1 +	(Point2-Point1) * ((W - (Point1|GetNormal()))/((Point2 - Point1)|GetNormal()));
		}

		inline bool operator==(const Plane& V) const
		{
			return (X == V.X) && (Y == V.Y) && (Z == V.Z) && (W == V.W);
		}

		inline bool operator!=(const Plane& V) const
		{
			return (X != V.X) || (Y != V.Y) || (Z != V.Z) || (W != V.W);
		}

		inline bool Equals(const Plane& V, float Tolerance) const
		{
			return (std::abs(X - V.X) < Tolerance) && (std::abs(Y - V.Y) < Tolerance) && (std::abs(Z - V.Z) < Tolerance) && (std::abs(W - V.W) < Tolerance);
		}

		inline float operator|(const Plane& V) const
		{
			return X * V.X + Y * V.Y + Z * V.Z + W * V.W;
		}

		inline Plane operator+(const Plane& V) const
		{
			return Plane(X + V.X, Y + V.Y, Z + V.Z, W + V.W);
		}

		inline Plane operator-(const Plane& V) const
		{
			return Plane(X - V.X, Y - V.Y, Z - V.Z, W - V.W);
		}

		inline Plane operator/(float Scale) const
		{
			const float RScale = 1.f / Scale;
			return Plane(X * RScale, Y * RScale, Z * RScale, W * RScale);
		}

		inline Plane operator*(float Scale) const
		{
			return Plane(X * Scale, Y * Scale, Z * Scale, W * Scale);
		}

		inline Plane operator*(const Plane& V)
		{
			return Plane (X * V.X, Y * V.Y, Z * V.Z, W * V.W);
		}

		inline Plane operator+=(const Plane& V)
		{
			X += V.X; Y += V.Y; Z += V.Z; W += V.W;
			return *this;
		}

		inline Plane operator-=(const Plane& V)
		{
			X -= V.X; Y -= V.Y; Z -= V.Z; W -= V.W;
			return *this;
		}

		inline Plane operator*=(float Scale)
		{
			X *= Scale; Y *= Scale; Z *= Scale; W *= Scale;
			return *this;
		}

		inline Plane operator*=(const Plane& V)
		{
			X *= V.X; Y *= V.Y; Z *= V.Z; W *= V.W;
			return *this;
		}

		inline Plane operator/=(float V)
		{
			const float RV = 1.f / V;
			X *= RV; Y *= RV; Z *= RV; W *= RV;
			return *this;
		}

	};
}