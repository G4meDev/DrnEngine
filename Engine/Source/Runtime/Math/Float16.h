#pragma once

#include <DirectXPackedVector.h>

namespace Drn
{
	class Float16
	{
	public:
		union
		{
			struct
			{
				uint16	Mantissa : 10;
				uint16	Exponent : 5;
				uint16	Sign : 1;
			} Components;

			uint16	Encoded;
		};

		inline Float16() : Encoded(0) {}
		inline Float16( const Float16& FP16Value ) { Encoded = FP16Value.Encoded; }
		inline Float16( float FP32Value ) { Set(FP32Value); }

		inline Float16& operator=(float FP32Value) { Set(FP32Value); return *this; };
		inline Float16& operator=(const Float16& FP16Value) { Encoded = FP16Value.Encoded; return *this; };
		inline operator float() const { return GetFloat(); };

		inline void Set(float FP32Value) { Encoded = DirectX::PackedVector::XMConvertFloatToHalf(FP32Value); };
		inline float GetFloat() const { return DirectX::PackedVector::XMConvertHalfToFloat(Encoded); };
	};
}