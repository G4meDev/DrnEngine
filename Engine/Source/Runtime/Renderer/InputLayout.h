#pragma once

#include "ForwardTypes.h"
#include <d3d12.h>

namespace Drn
{
	enum class EInputLayoutType : uint16
	{
		StandardMesh = 0,
		LineColorThickness,
		Position,

		MAX_TYPE
	};

	struct InputLayout_LineColorThickness
	{
	public:

		InputLayout_LineColorThickness(float InX, float InY, float InZ, uint8 InR, uint8 InG, uint8 InB, uint8 InA, float InThickness)
			: X(InX)
			, Y(InY)
			, Z(InZ)
			, R(InR)
			, G(InG)
			, B(InB)
			, A(InA)
			, Thickness(InThickness)
		{
		}

		InputLayout_LineColorThickness( const Vector& Pos, const Color& Color, float InThickness)
			: InputLayout_LineColorThickness(Pos.GetX(), Pos.GetY(), Pos.GetZ(), Color.R, Color.G, Color.B, Color.A, InThickness)
		{
		}

		InputLayout_LineColorThickness() : InputLayout_LineColorThickness(0, 0, 0, 1, 1, 1, 1, 0.08f)
		{
		}

		void Set(const Vector& Pos, const Color& Color, float InThickness)
		{
			X = Pos.GetX();
			Y = Pos.GetY();
			Z = Pos.GetZ();

			R = Color.R;
			G = Color.G;
			B = Color.B;
			A = Color.A;

			Thickness = InThickness;
		}

		float X;
		float Y;
		float Z;

		uint8 R;
		uint8 G;
		uint8 B;
		uint8 A;

		float Thickness;
	};

	struct InputLayout_StaticMesh
	{
	public:
		InputLayout_StaticMesh(float InX, float InY, float InZ, float InR, float InG, float InB, 
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

		InputLayout_StaticMesh()
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

		inline Vector GetPosition() { return Vector(X, Y, Z); }
		inline Vector GetNormal() { return Vector(N_X, N_Y, N_Z); }
		inline Vector GetTangent() { return Vector(T_X, T_Y, T_Z); }
		inline Vector GetBitTangent() { return Vector(BT_X, BT_Y, BT_Z); }
	};

	class InputLayout
	{
	public:
		static D3D12_INPUT_ELEMENT_DESC Position[1];
		static D3D12_INPUT_ELEMENT_DESC Color[2];
		static D3D12_INPUT_ELEMENT_DESC LineColorThickness[2];
		static D3D12_INPUT_ELEMENT_DESC StaticMesh[9];

		static D3D12_INPUT_LAYOUT_DESC GetLayoutDescriptionForType(EInputLayoutType Type);
		static std::string GetNameForType(EInputLayoutType Type);
	};
}