#pragma once

#include "Runtime/Core/Serializable.h"

namespace Drn
{
	class Color : public Serializable
	{
	public:
		inline Color(uint8_t InR, uint8_t InG, uint8_t InB, uint8_t InA = 255)
			: R(InR), G(InG), B(InB), A(InA)
		{
		}

		inline Color()
			: R(255), G(255), B(255), A(255)
		{
		}

		uint32_t& DWColor() {return *((uint32_t*)this);}
		const uint32_t& DWColor() const {return *((uint32_t*)this);}

		inline Color(uint32_t InColor)
		{
			DWColor() = InColor;
		}

		virtual void Serialize(Archive& Ar);

		inline bool operator==( const Color &C ) const
		{
			return DWColor() == C.DWColor();
		}

		inline bool operator!=( const Color& C ) const
		{
			return DWColor() != C.DWColor();
		}

		inline void operator+=(const Color& C)
		{
			R = (uint8_t) std::min((int32_t) R + (int32_t) C.R,255);
			G = (uint8_t) std::min((int32_t) G + (int32_t) C.G,255);
			B = (uint8_t) std::min((int32_t) B + (int32_t) C.B,255);
			A = (uint8_t) std::min((int32_t) A + (int32_t) C.A,255);
		}

		//FORCEINLINE std::string ToString() const
		//{
		//	return "";
		//	return std::string::Printf(TEXT("(R=%i,G=%i,B=%i,A=%i)"), R, G, B, A);
		//}

		static const Color White;
		static const Color Black;
		static const Color Transparent;
		static const Color Red;
		static const Color Green;
		static const Color Blue;
		static const Color Yellow;
		static const Color Cyan;
		static const Color Magenta;
		static const Color Orange;
		static const Color Purple;
		static const Color Turquoise;
		static const Color Silver;
		static const Color Emerald;

		uint8_t R;
		uint8_t G;
		uint8_t B;
		uint8_t A;
	};
}