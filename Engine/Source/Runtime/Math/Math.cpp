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

}

