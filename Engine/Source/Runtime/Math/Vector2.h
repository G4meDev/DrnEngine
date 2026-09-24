#pragma once

#include <DirectXMath.h>
#include <sstream>
#include <string>

using namespace DirectX;

namespace Drn
{
	class Vector2
	{
	public:
		inline Vector2(float X, float Y) { XMStoreFloat2(&m_Vector, XMVectorSet(X, Y, 0, 0)); }
		inline Vector2(float X) : Vector2(X, X) {}
		inline Vector2() : Vector2(0) {}

		inline Vector2( const Vector2& InVector ) { XMStoreFloat2(&m_Vector, InVector.Get()); }
		inline Vector2( const XMVECTOR& InVector ) { XMStoreFloat2(&m_Vector, InVector); }

		inline XMVECTOR Get() const { return XMLoadFloat2( &m_Vector ); }

		inline float GetX() const { return m_Vector.x; }
		inline float GetY() const { return m_Vector.y; }

		Vector2 GetSafeNormal(float Tolerance = KINDA_SMALL_NUMBER) const
		{	
			const float SquareSum = X*X + Y*Y;
			if(SquareSum > Tolerance)
			{
				const float Scale = 1.0f / std::sqrt(SquareSum);
				return Vector2(X*Scale, Y*Scale);
			}
			return Vector2(0.f, 0.f);
		}

		inline void Normalize(float Tolerance = KINDA_SMALL_NUMBER)
		{
			const float SquareSum = X*X + Y*Y;
			if(SquareSum > Tolerance)
			{
				const float Scale = 1.0f / std::sqrt(SquareSum);
				X *= Scale;
				Y *= Scale;
				return;
			}
			X = 0.0f;
			Y = 0.0f;
		}

		inline static float DotProduct(const Vector2& V1, const Vector2& V2)
		{
			return XMVectorGetX(XMVector2Dot(XMLoadFloat2(&V1.m_Vector), XMLoadFloat2(&V2.m_Vector)));
		}

		inline static float CrossProduct(const Vector2& V1, const Vector2& V2)
		{
			return V1.X*V2.Y - V1.Y*V2.X;
		}

		inline bool IsNearlyEqual(const Vector2& Other, float Tolerance = KINDA_SMALL_NUMBER) const
		{
			XMVECTOR Vec = XMLoadFloat2(&m_Vector);
			XMVECTOR OtherVec = XMLoadFloat2(&Other.m_Vector);
			return XMVector2NearEqual(Vec, OtherVec, XMVectorSet(Tolerance, Tolerance, Tolerance, Tolerance));
		}

#if WITH_EDITOR
		bool Draw(const std::string& id);
		bool Draw();
#endif

		static Vector2 ZeroVector;
		static Vector2 OneVector;

		union { struct { float X, Y; }; XMFLOAT2 m_Vector; };

	private:

		friend class Vector;
		friend class Quat;
		friend class Transform;
		friend class Matrix;
	};
}