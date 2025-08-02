#pragma once

#include <DirectXMath.h>

using namespace DirectX;

namespace Drn
{
	class Matrix
	{
	public:

		inline Matrix() { XMStoreFloat4x4( &m_Matrix, XMMatrixIdentity() ); }
		inline Matrix(const XMMATRIX& InMatrix) { XMStoreFloat4x4( &m_Matrix, InMatrix ); }
		Matrix(const Transform& InTransform);
		Matrix(const Vector& X, const Vector& Y, const Vector& Z, const Vector& W);

		inline XMMATRIX Get() const { return XMLoadFloat4x4( &m_Matrix ); }

		static Matrix MakeFromX( const Vector& XAxis);
		static Matrix MakeFromY( const Vector& YAxis);
		static Matrix MakeFromZ( const Vector& ZAxis);

		inline Matrix operator*( const Matrix& InMatrix ) const
		{
			return XMMatrixMultiply(Get(), InMatrix.Get());
		}

		static inline Matrix ScaleMatrix(const Vector& Scale)
		{
			return XMMatrixScalingFromVector(XMLoadFloat3(Scale.Get()));
		}

		static Matrix MatrixIdentity;

	private:

		XMFLOAT4X4 m_Matrix;

		friend class Transform;
		friend class Vector;
		friend class Quat;
	};
}