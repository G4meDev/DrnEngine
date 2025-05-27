#pragma once

#include <DirectXMath.h>
#include <string>
#include <sstream>

#include "Vector.h"

using namespace DirectX;

namespace Drn
{
	class Quat
	{
	public:

		inline Quat( float X, float Y, float Z, float W ) { XMStoreFloat4(&m_Vector, XMVectorSet(X, Y, Z, W)); }
		inline Quat( float Roll, float Pitch, float Yaw ) { XMStoreFloat4(&m_Vector, XMQuaternionRotationRollPitchYaw(Pitch, Yaw, Roll)); }
		inline Quat() { XMStoreFloat4(&m_Vector, XMQuaternionIdentity()); }

		inline Quat( const XMVECTOR& InVector ) { XMStoreFloat4(&m_Vector, InVector); }

		inline float GetX() const { return m_Vector.x; }
		inline float GetY() const { return m_Vector.y; }
		inline float GetZ() const { return m_Vector.z; }
		inline float GetW() const { return m_Vector.w; }

		inline Quat operator*(const Quat& Other) const
		{
			return XMQuaternionMultiply(Other.Get(), Get());
		}

		inline Quat Inverse() const { return Quat( XMQuaternionInverse(Get()) ); }
		inline Vector RotateVector( const Vector& InVector) { return Vector( XMVector3Rotate( XMLoadFloat3(&InVector.m_Vector), Get()) ); }
		inline Vector RotateVector( const Vector& InVector) const { return Vector( XMVector3Rotate( XMLoadFloat3(&InVector.m_Vector), Get()) ); }

		inline Vector InverseRotateVector(const Vector& InVector) const
		{
			XMVECTOR Inv = XMQuaternionInverse(Get());
			return XMVector3Rotate( XMLoadFloat3( &InVector.m_Vector ), Inv );
		}

		inline XMVECTOR Get() const { return XMLoadFloat4(&m_Vector); }

		inline static Quat Multiply( const Quat& Q1, const Quat& Q2 ) { return XMQuaternionMultiply(Q2.Get(), Q1.Get()); }

		inline Quat Normalize() { return Quat(XMQuaternionNormalize(Get())); }

		inline bool Equals( const Quat& Other, float Tolerance = KINDA_SMALL_NUMBER ) 
		{
			const XMVECTOR ToleranceV = XMVectorReplicatePtr( (const float*)(&Tolerance) );
			const XMVECTOR RotationSub = XMVectorAbs( Get() - Other.Get() );
			const XMVECTOR RotationAdd = XMVectorAbs( Get() + Other.Get() );

			uint32_t A = XMVector4GreaterR( RotationSub, ToleranceV );
			A = XMComparisonAnyTrue( A );

			uint32_t B = XMVector4GreaterR( RotationAdd, ToleranceV );
			B = XMComparisonAnyTrue( B );

			return !A || !B;
		}

		inline std::string ToString()
		{
			std::stringstream ss;
			ss << "(X: " << GetX() << ", Y: " << GetY() << ", Z: " << GetZ() << ", W: " << GetW() << ")";

			return ss.str();
		}

		static Quat Identity;

#if WITH_EDITOR
		void Draw(const std::string& id);
#endif

	private:
		XMFLOAT4 m_Vector;

		friend class Matrix;
	};
}