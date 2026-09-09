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
			Y.GetX(), Y.GetY(), Y.GetZ(), Y.GetW(),
			Z.GetX(), Z.GetY(), Z.GetZ(), Z.GetW(),
			W.GetX(), W.GetY(), W.GetZ(), W.GetW()
		));
	}

	//Matrix Matrix::MakeFromX( const Vector& XAxis )
	//{
	//	Vector const NewX = XAxis.GetSafeNormal();
	//	Vector const UpVector = ( Math::Abs(NewX.Y) < (1.f - KINDA_SMALL_NUMBER) ) ? Vector::UpVector : Vector::ForwardVector;
	//
	//	const Vector NewY = (UpVector ^ NewX).GetSafeNormal();
	//	const Vector NewZ = NewX ^ NewY;
	//
	//	return Matrix(NewX, NewY, NewZ, Vector::ZeroVector);
	//}

	Matrix Matrix::MakeFromY( const Vector& YAxis )
	{
		Vector const NewY = YAxis.GetSafeNormal();
		Vector const UpVector = ( Math::Abs(NewY.Y) < (1.f - KINDA_SMALL_NUMBER) ) ? Vector::UpVector : Vector::ForwardVector;

		const Vector NewZ = (UpVector ^ NewY).GetSafeNormal();
		const Vector NewX = NewY ^ NewZ;

		return Matrix(NewX, NewY, NewZ, Vector::ZeroVector);
	}

	Matrix Matrix::MakeFromZ( const Vector& ZAxis )
	{
		const Vector NewZ = ZAxis.GetSafeNormal();
		Vector UpVector = (Math::Abs(NewZ.Y) < (1.0f - KINDA_SMALL_NUMBER)) ? Vector::UpVector : Vector::ForwardVector;

		Vector NewX = (UpVector ^ NewZ).GetSafeNormal();
		Vector NewY = NewZ ^ NewX;

		return Matrix( NewX, NewY, NewZ, Vector::ZeroVector );
	}

	Matrix Matrix::MakeFromZY( const Vector& ZAxis, const Vector& YAxis )
	{
		Vector const NewZ = ZAxis.GetSafeNormal();
		Vector Norm = YAxis.GetSafeNormal();

		if ( Math::IsNearlyEqual(Math::Abs(NewZ | Norm), 1.f) )
		{
			Norm = ( Math::Abs(NewZ.Y) < (1.f - KINDA_SMALL_NUMBER) ) ? Vector::UpVector : Vector::ForwardVector;
		}

		const Vector NewX = (Norm ^ NewZ).GetSafeNormal();
		const Vector NewY = NewZ ^ NewX;

		return Matrix(NewX, NewY, NewZ, Vector::ZeroVector);
	}

	Matrix Matrix::TranslationMatrix( const Vector& Translation )
	{
		DirectX::XMMATRIX TranslationMat = DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&Translation.m_Vector));

		Matrix Result;
		XMStoreFloat4x4(&Result.m_Matrix, TranslationMat);
		return Result;
	}

}