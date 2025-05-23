#pragma once

namespace Drn
{
	class Vector4
	{
	public:
		inline Vector4(float X, float Y, float Z, float W) { m_Vector = XMVectorSet(X, Y, Z, W); }
		inline Vector4(Vector Vec, float W) { m_Vector = XMVectorSet(Vec.GetX(), Vec.GetY(), Vec.GetZ(), W); }
		inline Vector4(float X) : Vector4(X, X, X, X) {}
		inline Vector4() : Vector4(0) {}

		inline Vector4( const Vector4& InVector ) { m_Vector = InVector.m_Vector; }
		inline Vector4( const Vector& InVector ) { m_Vector = XMLoadFloat3(&InVector.m_Vector); }
		inline Vector4( const XMVECTOR& InVector ) { m_Vector = InVector; }

		inline const XMVECTOR* Get() { return &m_Vector; }

		inline float GetX() const { return XMVectorGetX( m_Vector ); }
		inline float GetY() const { return XMVectorGetY( m_Vector ); }
		inline float GetZ() const { return XMVectorGetZ( m_Vector ); }
		inline float GetW() const { return XMVectorGetW( m_Vector ); }


#if WITH_EDITOR
		bool Draw(const std::string& id);
		bool Draw();
#endif

	private:
		XMVECTOR m_Vector;

		friend class Quat;
		friend class Transform;
		friend class Matrix;
	};
}