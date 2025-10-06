#include "DrnPCH.h"
#include "Matrix.h"

namespace Drn
{
	Matrix Matrix::MatrixIdentity = Matrix();

	Matrix::Matrix( const Transform& InTransform )
	{
		DirectX::XMMATRIX ScaleMat			= DirectX::XMMatrixScalingFromVector(DirectX::XMLoadFloat3(&InTransform.Scale.m_Vector));
		DirectX::XMMATRIX RotationMat		= DirectX::XMMatrixRotationQuaternion(InTransform.Rotation.Get());
		DirectX::XMMATRIX TranslationMat	= DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&InTransform.Location.m_Vector));

		XMStoreFloat4x4(&m_Matrix, ScaleMat * RotationMat * TranslationMat);
	}

	Matrix::Matrix( const Vector& X, const Vector& Y, const Vector& Z, const Vector& W )
	{
		XMStoreFloat4x4(&m_Matrix, XMMatrixSet
		(
			X.GetX(), X.GetY(), X.GetZ(), 0,
			Y.GetX(), Y.GetY(), Y.GetZ(), 0,
			Z.GetX(), Z.GetY(), Z.GetZ(), 0,
			W.GetX(), W.GetY(), W.GetZ(), 1
		));
	}

	Matrix::Matrix( const Vector4& X, const Vector4& Y, const Vector4& Z, const Vector4& W )
	{
		XMStoreFloat4x4(&m_Matrix, XMMatrixSet
		(
			X.GetX(), X.GetY(), X.GetZ(), X.GetW(),
			Y.GetX(), Y.GetY(), Y.GetZ(), X.GetW(),
			Z.GetX(), Z.GetY(), Z.GetZ(), Z.GetW(),
			W.GetX(), W.GetY(), W.GetZ(), W.GetW()
		));
	}

	Matrix Matrix::MakeFromX( const Vector& XAxis )
	{
		XMVECTOR NewX = XMLoadFloat3( XAxis.Get() );
		NewX = XMVector3Normalize( NewX );

		XMVECTOR UpVector;

		if ( XMVector3IsInfinite(NewX) )
		{
			UpVector = XMVectorSet( 1, 0, 0, 0 );
		}

		else
		{
			UpVector = XMVectorSet( 0, 1, 0, 0 );
		}

		XMVECTOR NewY = XMVector3Cross( UpVector, NewX );
		NewY = XMVector3Normalize(NewY);

		XMVECTOR NewZ = XMVector3Cross( NewX, NewY );

		return Matrix( NewX, NewY, NewZ, Vector::ZeroVector );
	}

	Matrix Matrix::MakeFromY( const Vector& YAxis )
	{
		XMVECTOR NewY = XMLoadFloat3( YAxis.Get() );
		NewY = XMVector3Normalize( NewY );

		XMVECTOR UpVector;

		if ( XMVector3IsInfinite(NewY) )
		{
			UpVector = XMVectorSet( 0, 1, 0, 0 );
		}

		else
		{
			UpVector = XMVectorSet( 0, 0, 1, 0 );
		}

		XMVECTOR NewZ = XMVector3Cross( UpVector, NewY );
		NewZ = XMVector3Normalize(NewZ);

		XMVECTOR NewX = XMVector3Cross( NewY, NewZ );

		return Matrix( NewX, NewY, NewZ, Vector::ZeroVector );
	}

	Matrix Matrix::MakeFromZ( const Vector& ZAxis )
	{
		XMVECTOR NewZ = XMLoadFloat3( ZAxis.Get() );
		NewZ = XMVector3Normalize( NewZ );

		XMVECTOR UpVector;

		if ( XMVector3IsInfinite(NewZ) )
		{
			UpVector = XMVectorSet( 0, 0, 1, 0 );
		}

		else
		{
			UpVector = XMVectorSet( 0, 1, 0, 0 );
		}

		XMVECTOR NewX = XMVector3Cross( UpVector, NewZ );
		NewX = XMVector3Normalize(NewX);

		XMVECTOR NewY = XMVector3Cross( NewZ, NewX );

		return Matrix( NewX, NewY, NewZ, Vector::ZeroVector );
	}

	Matrix Matrix::TranslationMatrix( const Vector& Translation )
	{
		DirectX::XMMATRIX TranslationMat = DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&Translation.m_Vector));

		Matrix Result;
		XMStoreFloat4x4(&Result.m_Matrix, TranslationMat);
		return Result;
	}

}