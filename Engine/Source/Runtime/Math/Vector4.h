#pragma once

namespace Drn
{
	class Vector4
	{
	public:
		inline Vector4(float X, float Y, float Z, float W) { XMStoreFloat4(&m_Vector, XMVectorSet(X, Y, Z, W)); }
		inline Vector4(Vector Vec, float W) { XMStoreFloat4(&m_Vector, XMVectorSet(Vec.GetX(), Vec.GetY(), Vec.GetZ(), W)); }
		inline Vector4(float X) : Vector4(X, X, X, X) {}
		inline Vector4() : Vector4(0) {}

		inline Vector4( const Vector4& InVector ) { XMStoreFloat4(&m_Vector, InVector.Get()); }
		//inline Vector4( const Vector& InVector ) { XMStoreFloat4(&m_Vector, XMLoadFloat3(&InVector.m_Vector)); }
		inline Vector4( const XMVECTOR& InVector ) { XMStoreFloat4(&m_Vector, InVector); }

		inline XMVECTOR Get() const { return XMLoadFloat4( &m_Vector ); }

		inline float GetX() const { return m_Vector.x; }
		inline float GetY() const { return m_Vector.y; }
		inline float GetZ() const { return m_Vector.z; }
		inline float GetW() const { return m_Vector.w; }


#if WITH_EDITOR
		bool Draw(const std::string& id);
		bool Draw();
#endif

	private:
		XMFLOAT4 m_Vector;

		friend class Vector;
		friend class Quat;
		friend class Transform;
		friend class Matrix;
	};
}