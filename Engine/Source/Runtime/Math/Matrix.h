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

		inline XMMATRIX Get() { return m_Matrix; }

		inline Matrix operator*( const Matrix& InMatrix ) { return XMMatrixMultiply(m_Matrix, InMatrix.m_Matrix); }

		static Matrix MatrixIdentity;

	private:

		XMMATRIX m_Matrix;

		friend class Transform;
		friend class Vector;
		friend class Quat;
	};
}