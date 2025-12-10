#pragma once

#define SMALL_NUMBER		(1.e-8f)
#define KINDA_SMALL_NUMBER	(1.e-4f)
//#define PI 3.14159
#define PI_ON_180_DEGREES 0.0174532778 // PI / 180

#include "Runtime/Math/Vector.h"
#include <algorithm>

namespace Drn
{
	template <typename T>
	FORCEINLINE constexpr T Align(T Val, uint64 Alignment)
	{
		return (T)(((uint64)Val + Alignment - 1) & ~(Alignment - 1));
	}

	template <typename T>
	FORCEINLINE constexpr T AlignArbitrary(T Val, uint64 Alignment)
	{
		return (T)((((uint64)Val + Alignment - 1) / Alignment) * Alignment);
	}

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

		static Vector VInterpTo(const Vector& Current, const Vector& Target, float DeltaTime, float InterpSpeed);
		static Quat QInterpTo(const Quat& Current, const Quat& Target, float DeltaTime, float InterpSpeed);

		inline static int8 NormalizedFloatToInt8(float Value)
		{
			Value = std::clamp(Value, -1.0f, 1.0f);
			int32 Scaled = static_cast<int32>(std::round(Value * 127.0f));

			Scaled = std::clamp(Scaled, -128, 127);
			return static_cast<int8>(Scaled);
		}

		inline static float Int8ToNormalizedFloat( int8 Value )
		{
			return (float)Value/127.0f;
		}

		inline static uint32 PackSignedNormalizedVectorToUint32(const Vector& InVector)
		{
			Vector NormalizedVector = InVector.GetSafeNormal();
			int8 R = NormalizedFloatToInt8(NormalizedVector.GetX());
			int8 G = NormalizedFloatToInt8(NormalizedVector.GetY());
			int8 B = NormalizedFloatToInt8(NormalizedVector.GetZ());
			int8 A = NormalizedFloatToInt8(1.0f);

			uint32 Result = (uint32)(uint8)R | ( (uint32)(uint8)G << 8 ) | ( (uint32)(uint8)B << 16 ) | ( (uint32)(uint8)A << 24 );
			return Result;
		}

		inline static Vector UnpackUint32ToSignedNormalizedVector(uint32 Value)
		{
			int8 R = (int8)(Value & 0xFF);
			int8 G = (int8)((Value >> 8) & 0xFF);
			int8 B = (int8)((Value >> 16) & 0xFF);

			return Vector(Int8ToNormalizedFloat(R), Int8ToNormalizedFloat(G), Int8ToNormalizedFloat(B));
		}
	};
}