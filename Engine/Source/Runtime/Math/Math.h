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
	inline constexpr T Align(T Val, uint64 Alignment)
	{
		return (T)(((uint64)Val + Alignment - 1) & ~(Alignment - 1));
	}

	template <typename T>
	inline constexpr T AlignArbitrary(T Val, uint64 Alignment)
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

		static int32 Rand() { return std::rand(); }
		static float SRand();
		static inline float InvSqrt(float F) {  return 1.0f / std::sqrt(F); }
		static inline float TruncToInt(float F) { return std::trunc(F); }

		static float FInterpConstantTo(float Current, float Target, float DeltaTime, float InterpSpeed);
		static float FInterpTo(float Current, float Target, float DeltaTime, float InterpSpeed);
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

		inline static uint8 Quantize8UnsignedByte(float x)
		{
			int32 Ret = (int32)(x * 255.999f);
			return (uint8)Ret;
		}

		inline static uint8 Quantize8SignedByte(float x)
		{
			float y = x * 0.5f + 0.5f;
			return Quantize8UnsignedByte(y);
		}

		inline static uint32 ReverseBits(uint32 Bits)
		{
			Bits = ( Bits << 16) | ( Bits >> 16);
			Bits = ( (Bits & 0x00ff00ff) << 8 ) | ( (Bits & 0xff00ff00) >> 8 );
			Bits = ( (Bits & 0x0f0f0f0f) << 4 ) | ( (Bits & 0xf0f0f0f0) >> 4 );
			Bits = ( (Bits & 0x33333333) << 2 ) | ( (Bits & 0xcccccccc) >> 2 );
			Bits = ( (Bits & 0x55555555) << 1 ) | ( (Bits & 0xaaaaaaaa) >> 1 );
			return Bits;
		}

		template <class T>
		inline static T DivideAndRoundUp(T Dividend, T Divisor)
		{
			return (Dividend + Divisor - 1) / Divisor;
		}

		static inline uint32 CountLeadingZeros(uint32 Value)
		{
			unsigned long Log2;
			long Mask = -long(_BitScanReverse(&Log2, Value) != 0);
			return ((31 - Log2) & Mask) | (32 & ~Mask);
		}

		template<class T, class U>
		static inline T Lerp( const T& A, const T& B, const U& Alpha )
		{
			return (T)(A + (B-A) * Alpha);
		}

		template< class T, class U > 
		static inline T CubicInterp( const T& P0, const T& T0, const T& P1, const T& T1, const U& A )
		{
			const float A2 = A  * A;
			const float A3 = A2 * A;

			return (T)(P0 * ((2*A3)-(3*A2)+1)) + (T0 * (A3-(2*A2)+A)) + (T1 * (A3-A2)) + (P1 * ((-2*A3)+(3*A2)));
		}

		static inline int32 FloorToInt(float F)
		{
			return TruncToInt(floorf(F));
		}

		static inline int32 RoundToInt(float F)
		{
			return FloorToInt(F + 0.5f);
		}

		static inline bool IsNearlyZero(float Value, float ErrorTolerance = SMALL_NUMBER)
		{
			return std::abs(Value) <= ErrorTolerance;
		}

		template< class T > 
		static inline T Square( const T A )
		{
			return A*A;
		}

		template<class T>
		static T GetRangePct(T MinValue, T MaxValue, T Value)
		{
			static_assert(std::is_floating_point_v<T>);

			const T Divisor = MaxValue - MinValue;
			if (Math::IsNearlyZero(Divisor))
			{
				return (Value >= MaxValue) ? (T)1 : (T)0;
			}

			return (Value - MinValue) / Divisor;
		}

		template<class T>
		static FORCEINLINE T GetRangeValue(T const& RangeMin, T const& RangeMax, T Pct)
		{
			return Math::Lerp<T>(RangeMin, RangeMax, Pct);
		}

		template<class T>
		static T GetMappedRangeValueClamped(const T& InMin, const T& InMax, const T& OutMin, const T& OutMax, const T Value)
		{
			const T ClampedPct = std::clamp<T>(GetRangePct(InMin, InMax, Value), 0, 1);
			return GetRangeValue(OutMin, OutMax, ClampedPct);
		}
	};
}