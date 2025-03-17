#include "DrnPCH.h"
#include "Math.h"

LOG_DEFINE_CATEGORY(LogMath, "Math");

namespace Drn
{
	float Math::Mod(float A, float B)
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
}

