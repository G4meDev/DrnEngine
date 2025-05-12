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

		inline float GetX() const { return XMVectorGetX( XMLoadFloat3(&m_Vector) ); }
		inline float GetY() const { return XMVectorGetY( XMLoadFloat3(&m_Vector) ); }
		inline float GetZ() const { return XMVectorGetZ( XMLoadFloat3(&m_Vector) ); }
		inline float GetW() const { return XMVectorGetW( XMLoadFloat3(&m_Vector) ); }

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