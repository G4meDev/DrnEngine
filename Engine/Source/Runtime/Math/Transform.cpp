#include "DrnPCH.h"
#include "Transform.h"

namespace Drn
{
	Transform::Transform( const Matrix& InMatrix )
	{
		XMVECTOR DecompLocation;
		XMVECTOR DecompRotation;
		XMVECTOR DecompScale;

		XMMatrixDecompose(&DecompScale, &DecompRotation, &DecompLocation, InMatrix.m_Matrix);

		Location = Vector(DecompLocation);
		Rotation = Quat(DecompRotation);
		Scale = Vector(DecompScale);
	}

	Transform Transform::transform( const Transform& InTransform ) const
	{
		Matrix Mat1(*this);
		Matrix Mat2(InTransform);

		return Transform(Mat1 * Mat2);
	}

	Transform Transform::Invtransform( const Transform& InTransform )
	{
		Transform Result;

		return Result;
	}

	Transform Transform::GetRelativeTransform( const Transform& Other ) const
	{
		Transform Result;

		Vector VScale3D = Vector(XMVectorReciprocal( XMLoadFloat3(&Other.Scale.m_Vector)) );

		Vector VQTranslation = Location - Other.Location;

		Quat VInverseRot = Other.Rotation.Inverse();
		Vector VR = VInverseRot.RotateVector(VQTranslation);

		Vector VTranslation = VR * VScale3D;

		Quat VRotation = Quat::Multiply(VInverseRot, Rotation);

		Result.Scale	= VScale3D;
		Result.Rotation	= VRotation;
		Result.Location = VTranslation;

		return Result;
	}

}