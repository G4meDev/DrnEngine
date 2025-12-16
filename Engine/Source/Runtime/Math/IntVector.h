#pragma once

namespace Drn
{
	class IntVector
	{
	public:
		IntVector(int InX, int InY, int InZ) : X(InX), Y(InY), Z(InZ) {}
		IntVector(int Value) : IntVector(Value, Value, Value) {}
		IntVector() : IntVector(0) {}

		bool operator==( const IntVector& Other ) const;
		bool operator!=( const IntVector& Other ) const;

		inline int GetX() const { return X; }
		inline int GetY() const { return Y; }
		inline int GetZ() const { return Z; }


	public:
		int X;
		int Y;
		int Z;

		static IntVector Zero;
		static IntVector One;

	private:
	};
}