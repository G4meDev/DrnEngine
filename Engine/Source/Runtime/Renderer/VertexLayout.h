#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	enum class EInputLayoutType : uint16
	{
		Color = 0,
		LineColorThickness,

		MAX_TYPE
	};

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

	struct VertexData_LineColorThickness
	{
	public:

		VertexData_LineColorThickness(float InX, float InY, float InZ, float InR, float InG, float InB, float InThickness)
			: X(InX)
			, Y(InY)
			, Z(InZ)
			, R(InR)
			, G(InG)
			, B(InB)
			, Thickness(InThickness)
		{
		}

		VertexData_LineColorThickness( const Vector& Pos, const Vector& Color, float InThickness)
			: VertexData_LineColorThickness(Pos.GetX(), Pos.GetY(), Pos.GetZ(), Color.GetX(), Color.GetY(), Color.GetZ(), InThickness)
		{
		}

		VertexData_LineColorThickness() : VertexData_LineColorThickness(0, 0, 0, 1, 1, 1, 0.08f)
		{
		}

		float X;
		float Y;
		float Z;

		float R;
		float G;
		float B;

		float Thickness;
	};

	class VertexLayout
	{
	public:
		static D3D12_INPUT_ELEMENT_DESC Color[2];
		static D3D12_INPUT_ELEMENT_DESC LineColorThickness[2];

		static D3D12_INPUT_LAYOUT_DESC GetLayoutDescriptionForType(EInputLayoutType Type);
		static std::string GetNameForType(EInputLayoutType Type);
	};
}