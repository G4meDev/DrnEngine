#pragma once

#include <DirectXMath.h>
#include <string>
#include <sstream>

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
		inline const XMFLOAT3* Get() const { return &m_Vector; }

		inline float GetX() const { return m_Vector.x; }
		inline float GetY() const { return m_Vector.y; }
		inline float GetZ() const { return m_Vector.z; }

		inline Vector operator-( const Vector& other ) const
		{
			return Vector( XMVectorSubtract(
				XMLoadFloat3(&m_Vector), XMLoadFloat3(&other.m_Vector)) );
		}

		inline Vector operator+( const Vector& other ) const
		{
			return Vector( XMVectorAdd(
				XMLoadFloat3(&m_Vector), XMLoadFloat3(&other.m_Vector)) );
		}

		inline Vector operator*( const Vector& other )
		{
			return Vector( XMVectorMultiply(
				XMLoadFloat3(&m_Vector), XMLoadFloat3(&other.m_Vector)) );
		}

		inline Vector operator*( float Other ) const
		{
			return Vector( XMVectorMultiply(
				XMLoadFloat3(&m_Vector), XMVectorSet(Other, Other, Other, 0)) );
		}

		inline Vector operator*( float Other )
		{
			return Vector( XMVectorMultiply(
				XMLoadFloat3(&m_Vector), XMVectorSet(Other, Other, Other, 0)) );
		}

		inline Vector operator/( float Other )
		{
			return Vector( XMVectorDivide(
				XMLoadFloat3(&m_Vector), XMVectorSet(Other, Other, Other, 0)) );
		}
		
		inline bool Equals( const Vector& Other ) 
		{
			uint32_t Result;
			XMVectorEqualR(&Result, XMLoadFloat3(&m_Vector), XMLoadFloat3(&Other.m_Vector));
			return XMComparisonAllTrue(Result);
		}

		inline static float Distance(const Vector& V1, const Vector& V2) 
		{
			XMVECTOR Diff = XMVectorSubtract(XMLoadFloat3(&V1.m_Vector), XMLoadFloat3(&V2.m_Vector));
			return XMVectorGetX( XMVector3Length( Diff ) );
		}

		inline float GetMaxComponent() const 
		{
			XMVECTOR Vec = XMLoadFloat3(&m_Vector);

			return XMMax(XMMax(XMVectorGetX(Vec), XMVectorGetY(Vec)), XMVectorGetZ(Vec));
		}

		inline float GetMinComponent() const 
		{
			XMVECTOR Vec = XMLoadFloat3(&m_Vector);

			return XMMin(XMMin(XMVectorGetX(Vec), XMVectorGetY(Vec)), XMVectorGetZ(Vec));
		}

		inline float operator|( const Vector& V ) const
		{
			XMVECTOR Result = XMVector3Dot(XMLoadFloat3(Get()), XMLoadFloat3(V.Get()));
			return XMVectorGetX(Result);
		}

		static inline float DotProduct(const Vector& A, const Vector& B)
		{
			return A | B;
		}


		inline Vector operator^( const Vector& V ) const
		{
			XMVECTOR Result = XMVector3Cross(XMLoadFloat3(Get()), XMLoadFloat3(V.Get()));
			return Result;
		}

		static inline Vector CrossProduct(const Vector& A, const Vector& B)
		{
			return A ^ B;
		}


		static Vector FromU32(uint32_t Value)
		{
			uint8_t X = 255;
			uint8_t Y = 255;
			uint8_t Z = 255;

			X &= Value; 
			Y &= (Value >> 8); 
			Z &= (Value >> 16); 

			return Vector( float( X ) / 255.0f, float( Y ) / 255.0f, float( Z ) / 255.0f );
		}

		inline bool IsZero() const
		{
			XMVECTOR Vec = XMLoadFloat3(&m_Vector);

			return XMVectorGetX(Vec) == 0.0f && XMVectorGetY(Vec) == 0.0f && XMVectorGetZ(Vec) == 0.0f;
		}

		inline Vector GetSafeNormal( float Tolerance = SMALL_NUMBER) const
		{
			const float SquareSum = m_Vector.x * m_Vector.x + m_Vector.y * m_Vector.y + m_Vector.z * m_Vector.z;

			if (SquareSum == 1.0f)
			{
				return *this;
			}
			else if (SquareSum < Tolerance)
			{
				return Vector::ZeroVector;
			}

			const float Scale = 1.0f / std::sqrt(SquareSum);
			return Vector(m_Vector.x * Scale, m_Vector.y * Scale, m_Vector.z * Scale);
		}

		inline void FindBestAxisVectors(Vector& Axis1, Vector& Axis2) const
		{
			const float NX = std::abs(m_Vector.x);
			const float NY = std::abs(m_Vector.y);
			const float NZ = std::abs(m_Vector.z);

			if( NZ>NX && NZ>NY )	Axis1 = Vector(1,0,0);
			else					Axis1 = Vector(0,0,1);

			Axis1 = (Axis1 - *this * (Axis1 | *this)).GetSafeNormal();
			Axis2 = Axis1 ^ *this;
		}

		inline std::string ToString()
		{
			XMVECTOR Vec = XMLoadFloat3(&m_Vector);
			std::stringstream ss;
			ss << "(X: " << XMVectorGetX(Vec) << ", Y: " << XMVectorGetY(Vec) << ", Z: " << XMVectorGetZ(Vec) << ")";

			return ss.str();
		}

		static Vector ZeroVector;
		static Vector OneVector;
		static Vector UpVector;
		static Vector DownVector;
		static Vector RightVector;
		static Vector LeftVector;
		static Vector ForwardVector;
		static Vector BackwardVector;

#if WITH_EDITOR
		void Draw(const std::string& id);
#endif

	private:
		XMFLOAT3 m_Vector;

		friend class Vector4;
		friend class Quat;
		friend class Transform;
		friend class Matrix;
	};
}