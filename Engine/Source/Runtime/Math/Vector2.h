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

#if WITH_EDITOR
		bool Draw(const std::string& id);
		bool Draw();
#endif

		static Vector ZeroVector;
		static Vector OneVector;

		union { struct { float X, Y; }; XMFLOAT2 m_Vector; };

	private:

		friend class Vector;
		friend class Quat;
		friend class Transform;
		friend class Matrix;
	};
}