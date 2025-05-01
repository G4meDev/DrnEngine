#pragma once

#include "ForwardTypes.h"
#include <d3d12.h>

namespace Drn
{
	enum class EInputLayoutType : uint16
	{
		Color = 0,
		LineColorThickness,
		StandardMesh,

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

	struct VertexData_StaticMesh
	{
	public:
		VertexData_StaticMesh(float InX, float InY, float InZ, float InR, float InG, float InB, 
			float InN_X, float InN_Y, float InN_Z, float InT_X, float InT_Y, float InT_Z,
			float InBT_X, float InBT_Y, float InBT_Z, float InU1, float InV1, float InU2, float InV2,
			float InU3, float InV3, float InU4, float InV4)
			: X(InX)
			, Y(InY)
			, Z(InZ)
			, R(InR)
			, G(InG)
			, B(InB)
			, N_X(InN_X)
			, N_Y(InN_Y)
			, N_Z(InN_Z)
			, T_X(InT_X)
			, T_Y(InT_Y)
			, T_Z(InT_Z)
			, BT_X(InBT_X)
			, BT_Y(InBT_Y)
			, BT_Z(InBT_Z)
			, U1(InU1)
			, V1(InV1)
			, U2(InU2)
			, V2(InV2)
			, U3(InU3)
			, V3(InV3)
			, U4(InU4)
			, V4(InV4)
		{
		}

		VertexData_StaticMesh()
		{
		}

		float X;
		float Y;
		float Z;

		float R;
		float G;
		float B;

		float N_X;
		float N_Y;
		float N_Z;

		float T_X;
		float T_Y;
		float T_Z;

		float BT_X;
		float BT_Y;
		float BT_Z;

		float U1;
		float V1;

		float U2;
		float V2;

		float U3;
		float V3;

		float U4;
		float V4;
	};

	class VertexLayout
	{
	public:
		static D3D12_INPUT_ELEMENT_DESC Color[2];
		static D3D12_INPUT_ELEMENT_DESC LineColorThickness[2];
		static D3D12_INPUT_ELEMENT_DESC StaticMesh[9];

		static D3D12_INPUT_LAYOUT_DESC GetLayoutDescriptionForType(EInputLayoutType Type);
		static std::string GetNameForType(EInputLayoutType Type);
	};
}