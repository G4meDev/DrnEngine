#pragma once

#include <DirectXMath.h>
#include <string>
#include <sstream>

#include "Vector.h"
#include "Runtime/Math/Rotator.h"
#include "Editor/EditorTypes.h"

using namespace DirectX;

namespace Drn
{
	class Quat
	{
	public:

		inline Quat( float X, float Y, float Z, float W ) { XMStoreFloat4(&m_Vector, XMVectorSet(X, Y, Z, W)); }
		inline Quat( float Roll, float Pitch, float Yaw ) { XMStoreFloat4(&m_Vector, XMQuaternionRotationRollPitchYaw(Pitch, Yaw, Roll)); }
		inline Quat( const Vector& Axis, float Angle ) { XMStoreFloat4(&m_Vector, XMQuaternionRotationAxis(XMLoadFloat3(Axis.Get()), Angle)); }
		inline Quat() { XMStoreFloat4(&m_Vector, XMQuaternionIdentity()); }

		inline Quat( const XMVECTOR& InVector ) { XMStoreFloat4(&m_Vector, InVector); }
		inline Quat( const Vector& InVector ) : Quat(InVector.GetX(), InVector.GetY(), InVector.GetZ()) {};

		//Rotator ToRotator() const;

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

		inline void Normalize() { *this = Quat(XMQuaternionNormalize(Get())); }

		inline bool Equals( const Quat& Other, float Tolerance = KINDA_SMALL_NUMBER ) const
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

		inline static Quat Slerp(const Quat& Q1, const Quat& Q2, float Slerp)
		{
			return XMQuaternionSlerp(Q1.Get(), Q2.Get(), Slerp);
		}

		inline Quat GetNormalized() { return XMQuaternionNormalize(XMLoadFloat4(&m_Vector)); }

		inline float AngularDistance(const Quat& Q)
		{
			float Product = XMVectorGetX( XMVector4Dot(Get(), Q.Get()) );
			return std::acos((2 * Product * Product) - 1.0f);
		}

		static Quat FromToRotation(const Vector& FromDir, const Vector& ToDir)
		{
			const XMVECTOR F = XMVector3Normalize(XMLoadFloat3(&FromDir.m_Vector));
			const XMVECTOR T = XMVector3Normalize(XMLoadFloat3(&ToDir.m_Vector));

			const float dot = XMVectorGetX(XMVector3Dot(F, T));
			if (dot >= 1.f)
			{
				return Quat::Identity;
			}
			else if (dot <= -1.f)
			{
				XMVECTOR axis = XMVector3Cross(F, XMLoadFloat3(&Vector::RightVector.m_Vector));
				if (XMVector3NearEqual(XMVector3LengthSq(axis), g_XMZero, g_XMEpsilon))
				{
					axis = XMVector3Cross(F, XMLoadFloat3(&Vector::UpVector.m_Vector));
				}

				return XMQuaternionRotationAxis(axis, XM_PI);
			}
			else
			{
				const XMVECTOR C = XMVector3Cross(F, T);
				XMFLOAT4 Temp;
				XMStoreFloat4(&Temp, C);

				const float s = sqrtf((1.f + dot) * 2.f);
				Temp.x /= s;
				Temp.y /= s;
				Temp.z /= s;
				Temp.w = s * 0.5f;

				return Quat(XMLoadFloat4(&Temp));
			}
		}

		static Quat LookRotation(const Vector& Forward, const Vector& Up)
		{
			Quat Q1 = FromToRotation(Vector::ForwardVector, Forward);
			
			const XMVECTOR C = XMVector3Cross(XMLoadFloat3(&Forward.m_Vector), XMLoadFloat3(&Up.m_Vector));
			if (XMVector3NearEqual(XMVector3LengthSq(C), g_XMZero, g_XMEpsilon))
			{
				return Q1;
			}
			
			const XMVECTOR U = XMQuaternionMultiply(XMLoadFloat4(&Q1.m_Vector), XMLoadFloat3(&Vector::UpVector.m_Vector));
			Quat Q2 = FromToRotation(U, Up);
			
			return XMQuaternionMultiply(XMLoadFloat4(&Q2.m_Vector), XMLoadFloat4(&Q1.m_Vector));
		}

		static Quat FromX(const Vector& XAxis)
		{
			return FromToRotation(Vector::RightVector, XAxis);
		}

		static Quat FromY(const Vector& YAxis)
		{
			return FromToRotation(Vector::UpVector, YAxis);
		}

		static Quat FromZ(const Vector& ZAxis)
		{
			return FromToRotation(Vector::ForwardVector, ZAxis);
		}

		inline Vector GetAxisX() const { return RotateVector(Vector(1, 0, 0)); }
		inline Vector GetAxisY() const { return RotateVector(Vector(0, 1, 0)); }
		inline Vector GetAxisZ() const { return RotateVector(Vector(0, 0, 1)); }

		inline Vector GetForwardAxis() const { return GetAxisZ(); }
		inline Vector GetRightAxis() const { return GetAxisX(); }
		inline Vector GetUpAxis() const { return GetAxisY(); }
		inline Vector GetVector() const { return GetAxisZ(); }

		std::string ToString();
		bool FromString(const std::string& Str);


		static Quat Identity;
		static Quat CubeFaceOrientation[6];

#if WITH_EDITOR
		bool Draw(const std::string& id, const std::string& Label = "", EParameterPopupContext PopupOptions = EParameterPopupContext::CopyPaste);
#endif

	private:
		XMFLOAT4 m_Vector;

		friend class Matrix;
	};
}