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

		inline Vector GetColumn( int i ) const { return Vector(m_Matrix.m[0][i], m_Matrix.m[1][i], m_Matrix.m[2][i]); }
		inline Vector GetRow( int i ) const { return Vector(m_Matrix.m[i][0], m_Matrix.m[i][1], m_Matrix.m[i][2]); }

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

		inline Vector4 TransformVector4(const Vector4& V) const
		{
			return XMVector4Transform(V.Get(), XMLoadFloat4x4(&m_Matrix));
		}

		inline Vector4 TransformVector(const Vector& V) const
		{
			return XMVector3Transform(XMLoadFloat3(V.Get()), XMLoadFloat4x4(&m_Matrix));
		}

		inline Matrix GetTranspose() const
		{
			return XMMatrixTranspose(XMLoadFloat4x4(&m_Matrix));
		}

		static Matrix MatrixIdentity;
		XMFLOAT4X4 m_Matrix;

	private:

		friend class Transform;
		friend class Vector;
		friend class Quat;
	};
}