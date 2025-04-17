#pragma once

#include <DirectXMath.h>

using namespace DirectX;

namespace Drn
{
	class Vector
	{
	public:

		inline Vector(float X, float Y, float Z) { XMStoreFloat3(&m_Vector, XMVectorSet(X, Y, Z, 0)); }
		inline Vector(float X) : Vector(X, X, X) {}
		inline Vector() : Vector(0) {}

		inline Vector( const XMVECTOR& InVector ) { XMStoreFloat3(&m_Vector, InVector); }

		inline const XMFLOAT3* Get() { return &m_Vector; }

		inline float GetX() const { return XMVectorGetX( XMLoadFloat3(&m_Vector) ); }
		inline float GetY() const { return XMVectorGetY( XMLoadFloat3(&m_Vector) ); }
		inline float GetZ() const { return XMVectorGetZ( XMLoadFloat3(&m_Vector) ); }

		inline Vector operator-( const Vector& other ) const
		{
			return Vector( XMVectorSubtract(
				XMLoadFloat3(&m_Vector), XMLoadFloat3(&other.m_Vector)) );
		}

		inline Vector operator*( const Vector& other )
		{
			return Vector( XMVectorMultiply(
				XMLoadFloat3(&m_Vector), XMLoadFloat3(&other.m_Vector)) );
		}
		
		inline bool Equals( const Vector& Other ) 
		{
			uint32_t Result;
			XMVectorEqualR(&Result, XMLoadFloat3(&m_Vector), XMLoadFloat3(&Other.m_Vector));
			return XMComparisonAllTrue(Result);
		}

		inline static Vector Zero() { return ZeroVector; }
		inline static Vector One() { return OneVector; }

	private:
		XMFLOAT3 m_Vector;

		static Vector ZeroVector;
		static Vector OneVector;

		friend class Quat;
		friend class Transform;
		friend class Matrix;
	};
}