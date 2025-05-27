#include "DrnPCH.h"
#include "Transform.h"

#include "Math.h"

namespace Drn
{
	Transform::Transform( const Matrix& InMatrix )
	{
		XMVECTOR DecompLocation;
		XMVECTOR DecompRotation;
		XMVECTOR DecompScale;

		XMMatrixDecompose(&DecompScale, &DecompRotation, &DecompLocation, InMatrix.Get());

		Location = Vector(DecompLocation);
		Rotation = Quat(DecompRotation);
		Scale = Vector(DecompScale);
	}

	Transform Transform::operator*( const Transform& Other ) const
	{
		Transform Result;

		Quat Q1 = this->Rotation;
		Quat Q2 = Other.Rotation;
		Vector T1 = this->Location;
		Vector T2 = Other.Location;
		Vector S1 = this->Scale;
		Vector S2 = Other.Scale;

		Result.Rotation = Quat::Multiply(Q2, Q1);

		Vector ScaledTransA = T1 * S2;
		Vector RotatedTranslate = Q2.RotateVector(ScaledTransA);
		Result.Location = RotatedTranslate + T2;

		Result.Scale = S1 * S2;

		return Result;
	}

	Transform Transform::GetRelativeTransform( const Transform& Other ) const
	{
		Transform Result;

		Vector VSafeScale3D = GetSafeScaleReciprocal( XMLoadFloat3( &Other.Scale.m_Vector ) );
		Vector VScale3D = VSafeScale3D * GetScale();

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

	Vector Transform::InverseTransformPosition( const Vector& InVector ) const
	{
		Vector TranslatedVector = InVector - Location;

		Vector VR =  Rotation.InverseRotateVector( TranslatedVector );
		Vector SafeReciprocal = GetSafeScaleReciprocal(Scale);

		Vector Result = VR * SafeReciprocal;
		return Result;
	}

	Vector Transform::GetSafeScaleReciprocal( const Vector& InScale ) const
	{
		XMVECTOR SafeReciprocalScale;
		XMVECTOR InScaleXmVec = XMLoadFloat3(&InScale.m_Vector);

		float smallnum = 1.e-8f;

		const XMVECTOR ReciprocalScale = XMVectorReciprocal( InScaleXmVec );
		const XMVECTOR Tolerance = XMVectorSet(smallnum, smallnum, smallnum, smallnum);
		const XMVECTOR ScaleZeroMask = XMVectorGreaterOrEqual( Tolerance, XMVectorAbs( InScaleXmVec ) );

		SafeReciprocalScale = XMVectorSelect( ReciprocalScale, XMVectorZero(), ScaleZeroMask );

		return SafeReciprocalScale;
	}

}