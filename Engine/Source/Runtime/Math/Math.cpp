#include "DrnPCH.h"
#include "Math.h"

LOG_DEFINE_CATEGORY(LogMath, "Math");

namespace Drn
{
	float Math::PI = 3.1415926535897932f;

	float Math::Mod( float A, float B )
	{
		const float AbsB = Abs(B);

		if (B < SMALL_NUMBER)
		{
			LOG(LogMath, Warning, "Can't mod on '0'.")
				return 0.0;
		}

		const double DA = double(A);
		const double DB = double(B);

		const double Div = DA / DB;
		const double IntPortion = TruncToDouble(Div) * DB;
		const double Result = DA - IntPortion;

		return float(Result);
	}


	float Math::Abs(float A)
	{
		return fabsf(A);
	}

	double Math::TruncToDouble(double A)
	{
		return trunc(A);
	}

	bool Math::IsNearlyEqual(float A, float B, float Telorance /*= 0.001f*/)
	{
		float Delta = A - B;
		return Delta > -Telorance && Delta < Telorance;
	}

	float Math::SRand()
	{
		return ((double)std::rand()) / RAND_MAX;
	}

	float Math::FInterpConstantTo( float Current, float Target, float DeltaTime, float InterpSpeed )
	{
		const float Dist = Target - Current;

		if( (Dist * Dist) < SMALL_NUMBER )
		{
			return Target;
		}

		const float Step = InterpSpeed * DeltaTime;
		return Current + Math::Clamp<float>(Dist, -Step, Step);
	}

	float Math::FInterpTo( float Current, float Target, float DeltaTime, float InterpSpeed )
	{
		if( InterpSpeed <= 0.f )
		{
			return Target;
		}

		const float Dist = Target - Current;

		if( (Dist * Dist) < SMALL_NUMBER )
		{
			return Target;
		}

		const float DeltaMove = Dist * Math::Clamp<float>(DeltaTime * InterpSpeed, 0.f, 1.f);
		return Current + DeltaMove;
	}

	Vector Math::VInterpTo( const Vector& Current, const Vector& Target, float DeltaTime, float InterpSpeed )
	{
		if (InterpSpeed <= 0.0f)
		{
			return Target;
		}

		const Vector Dist = Target - Current;
		if (Dist.SizeSquared() < KINDA_SMALL_NUMBER)
		{
			return Target;
		}

		const Vector Delta = Dist * std::clamp(DeltaTime * InterpSpeed, 0.0f, 1.0f);
		return Current + Delta;
	}

	Quat Math::QInterpTo( const Quat& Current, const Quat& Target, float DeltaTime, float InterpSpeed )
	{
		if (InterpSpeed <= 0.0f)
		{
			return Target;
		}

		if (Current.Equals(Target))
		{
			return Target;
		}

		return Quat::Slerp( Current, Target, std::clamp(DeltaTime * InterpSpeed, 0.0f, 1.0f) );
	}

	Vector2 Math::ComputeBarycentricInPlane( const Vector& P0, const Vector& P1, const Vector& P2, const Vector& P )
	{
		Vector2 Bary;
		Vector P10 = P1 - P0;
		Vector P20 = P2 - P0;
		Vector PP0 = P - P0;
		float Size10 = P10.SizeSquared();
		float Size20 = P20.SizeSquared();
		float ProjSides = Vector::DotProduct(P10, P20);
		float ProjP1 = Vector::DotProduct(PP0, P10);
		float ProjP2 = Vector::DotProduct(PP0, P20);
		float Denom = Size10 * Size20 - ProjSides * ProjSides;
		Bary.X = (Size20 * ProjP1 - ProjSides * ProjP2) / Denom;
		Bary.Y = (Size10 * ProjP2 - ProjSides * ProjP1) / Denom;
		return Bary;
	}

	const Vector Math::FindClosestPointOnLineSegment( const Vector& P0, const Vector& P1, const Vector& P )
	{
		const Vector P10 = P1 - P0;
		const Vector PP0 = P - P0;
		const float Proj = Vector::DotProduct(P10, PP0);
		if (Proj < 0.0f)
		{
			return P0;
		}

		const float Denom2 = P10.SizeSquared();
		if (Denom2 < KINDA_SMALL_NUMBER)
		{
			return P0;
		}

		const float NormalProj = Proj / Denom2;
		if (NormalProj > 1.0)
		{
			return P1;
		}

		return P0 + P10 * NormalProj;
	}

	Vector Math::FindClosestPointOnTriangle( const Vector& P0, const Vector& P1, const Vector& P2, const Vector& P )
	{
		const float Epsilon = KINDA_SMALL_NUMBER;

		const Vector2 Bary = ComputeBarycentricInPlane(P0, P1, P2, P);

		if (Bary.X >= -Epsilon && Bary.X <= 1 + Epsilon && Bary.Y >= -Epsilon && Bary.Y <= 1 + Epsilon && (Bary.X + Bary.Y) <= (1 + Epsilon))
		{
			return P;
		}

		const Vector P10Closest = FindClosestPointOnLineSegment(P0, P1, P);
		const Vector P20Closest = FindClosestPointOnLineSegment(P0, P2, P);
		const Vector P21Closest = FindClosestPointOnLineSegment(P1, P2, P);

		const float P10Dist2 = (P - P10Closest).SizeSquared();
		const float P20Dist2 = (P - P20Closest).SizeSquared();
		const float P21Dist2 = (P - P21Closest).SizeSquared();

		if (P10Dist2 < P20Dist2)
		{
			if (P10Dist2 < P21Dist2)
			{
				return P10Closest;
			}
			else
			{
				return P21Closest;
			}
		}
		else
		{
			if (P20Dist2 < P21Dist2)
			{
				return P20Closest;
			}
			else
			{
				return P21Closest;
			}
		}
	}

	bool Math::PointOverlapsTriangle( const Vector& P0, const Vector& P1, const Vector& P2, const Vector& P, float Thickness )
	{
		const Vector ClosestPoint = FindClosestPointOnTriangle(P0, P1, P2, P);
		const float AdjustedThickness = std::max(Thickness, KINDA_SMALL_NUMBER);
		return (P - ClosestPoint).SizeSquared() <= (AdjustedThickness * AdjustedThickness);
	}

        }

