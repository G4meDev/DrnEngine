#pragma once

#include <DirectXMath.h>

#include "Vector.h"

using namespace DirectX;

namespace Drn
{
	class Quat
	{
	public:

		inline Quat( float X, float Y, float Z, float W ) { m_Vector = XMVectorSet(X, Y, Z, W); }
		inline Quat() { m_Vector = XMQuaternionIdentity(); }

		inline Quat( const XMVECTOR& InVector ) { m_Vector = InVector; }

		inline float GetX() const { return XMVectorGetX( m_Vector ); }
		inline float GetY() const { return XMVectorGetY( m_Vector ); }
		inline float GetZ() const { return XMVectorGetZ( m_Vector ); }
		inline float GetW() const { return XMVectorGetW( m_Vector ); }

		inline Quat Inverse() const { return Quat( XMQuaternionInverse(m_Vector) ); }
		inline Vector RotateVector( const Vector& InVector) { return Vector( XMVector3Rotate( XMLoadFloat3(&InVector.m_Vector), m_Vector) ); }

		inline XMVECTOR Get() { return m_Vector; }

		inline static Quat Multiply( const Quat& Q1, const Quat& Q2 ) { return XMQuaternionMultiply(Q2.m_Vector, Q1.m_Vector); }

		inline Quat Normalize() { return Quat(XMQuaternionNormalize(m_Vector)); }

		inline bool Equals( const Quat& Other ) 
		{
			uint32_t Result;
			XMVectorEqualR(&Result, m_Vector, Other.m_Vector);
			return XMComparisonAllTrue(Result);
		}

	private:
		XMVECTOR m_Vector;

		friend class Matrix;
	};
}