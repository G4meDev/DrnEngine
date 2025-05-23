#pragma once

#include <DirectXMath.h>

using namespace DirectX;

namespace Drn
{
	class Matrix
	{
	public:

		inline Matrix() { m_Matrix = XMMatrixIdentity(); }
		inline Matrix(const XMMATRIX& InMatrix) { m_Matrix = InMatrix; }
		Matrix(const Transform& InTransform);
		Matrix(const Vector& X, const Vector& Y, const Vector& Z, const Vector& W);

		inline XMMATRIX Get() const { return m_Matrix; }

		static Matrix MakeFromX( const Vector& XAxis);
		static Matrix MakeFromY( const Vector& YAxis);
		static Matrix MakeFromZ( const Vector& ZAxis);

		inline Matrix operator*( const Matrix& InMatrix ) { return XMMatrixMultiply(m_Matrix, InMatrix.m_Matrix); }

		static Matrix MatrixIdentity;

	private:

		XMMATRIX m_Matrix;

		friend class Transform;
		friend class Vector;
		friend class Quat;
	};
}