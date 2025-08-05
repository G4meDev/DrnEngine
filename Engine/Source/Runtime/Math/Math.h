#pragma once

#define SMALL_NUMBER		(1.e-8f)
#define KINDA_SMALL_NUMBER	(1.e-4f)
//#define PI 3.14159
#define PI_ON_180_DEGREES 0.0174532778 // PI / 180

#include <algorithm>

namespace Drn
{
	class Math
	{
	public:

		static float PI;

		static float Mod(float A, float B);
		static float Abs(float A);
		static 	double TruncToDouble(double A);

		template<typename T>
		static T DegreesToRadians(T const& Degrees)
		{
			return Degrees * (T)PI_ON_180_DEGREES;
		}

		template<typename T>
		static T Clamp(T Value, T Min, T Max)
		{
			return (Value < Min) ? Min : (Value < Max) ? Value : Max;
		}

		static float Sin(float Value)
		{
			return sinf(Value);
		}

		static double Sin(double Value)
		{
			return sin(Value);
		}

		static float Cos(float Value)
		{
			return cosf(Value);
		}

		static double Cos(double Value)
		{
			return cos(Value);
		}

		static void SinCos(float* SinRes, float* CosRes, float Value)
		{
			*SinRes = Sin(Value);
			*CosRes = Cos(Value);
		}

		static float Tan(float Value)
		{
			return tanf(Value);
		}

		static bool IsNearlyEqual(float A, float B, float Telorance = 0.001f);

		template<typename T>
		static T Min(const T A, const T B)
		{
			return (A <= B) ? A : B;
		}

		template<typename T>
		static T Max(const T A, const T B)
		{
			return (A >= B) ? A : B;
		}

		template<typename T>
		static T ComponentWiseMin(const T A, const T B)
		{
			return T::ComponentWiseMin(A, B);
		}

		template<typename T>
		static T ComponentWiseMax(const T A, const T B)
		{
			return T::ComponentWiseMax(A, B);
		}

		static int32 RoundUpToPowerOfTwo(int32 Value)
		{
			int32 Exp = std::floor(std::log2(Value)) + 1;
			return std::pow(2, Exp);
		}

		static int32 RoundDownToPowerOfTwo(int32 Value)
		{
			int32 Exp = std::floor(std::log2(Value));
			return std::pow(2, Exp);
		}
	};
}