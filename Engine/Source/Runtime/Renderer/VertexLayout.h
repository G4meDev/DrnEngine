#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	struct VertexData_Color
	{
	public:

		VertexData_Color(float InX, float InY, float InZ, float InR, float InG, float InB)
			: X(InX)
			, Y(InY)
			, Z(InZ)
			, R(InR)
			, G(InG)
			, B(InB)
		{
		}

		VertexData_Color(const Vector& Pos, const Vector& Color)
			: VertexData_Color(Pos.GetX(), Pos.GetY(), Pos.GetZ(), Color.GetX(), Color.GetY(), Color.GetZ())
		{
		}

		VertexData_Color() : VertexData_Color(0, 0, 0, 1, 1, 1) 
		{
		}

		float X;
		float Y;
		float Z;

		float R;
		float G;
		float B;
	};

	class VertexLayout
	{
	public:
		static D3D12_INPUT_ELEMENT_DESC Color[2];
	};
}