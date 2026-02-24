#include "DrnPCH.h"
#include "BoxSphereBounds.h"

namespace Drn
{
	BoxSphereBounds BoxSphereBounds::operator+( const BoxSphereBounds& Other ) const
	{
		Box BoundingBox(this->Origin - this->BoxExtent, this->Origin + this->BoxExtent);

		BoundingBox += (Other.Origin - Other.BoxExtent);
		BoundingBox += (Other.Origin + Other.BoxExtent);

		BoxSphereBounds Result(BoundingBox);
		Result.SphereRadius = std::min(Result.SphereRadius, std::max((Origin - Result.Origin).Length() + SphereRadius, (Other.Origin - Result.Origin).Length() + Other.SphereRadius));

		return Result;
	}

	BoxSphereBounds BoxSphereBounds::TranslateBy( const Vector& Offset )
	{
		return BoxSphereBounds(Origin + Offset, BoxExtent, SphereRadius);
	}

	BoxSphereBounds BoxSphereBounds::TransformBy( const Transform& T )
	{
		BoxSphereBounds Result;
		XMMATRIX M = Matrix(T).Get();
		M.r[0];

		XMVECTOR m0 = M.r[0];
		XMVECTOR m1 = M.r[1];
		XMVECTOR m2 = M.r[2];
		XMVECTOR m3 = M.r[3];

		XMVECTOR NewOrigin = XMVectorReplicate(Origin.GetX()) * m0;
		NewOrigin = XMVectorReplicate(Origin.GetY()) * m1 + NewOrigin;
		NewOrigin = XMVectorReplicate(Origin.GetZ()) * m2 + NewOrigin;
		NewOrigin = NewOrigin + m3;

		XMVECTOR NewExtent = XMVectorAbs(XMVectorReplicate(BoxExtent.GetX()) * m0);
		NewExtent = NewExtent + XMVectorAbs(XMVectorReplicate(BoxExtent.GetY()) * m1);
		NewExtent = NewExtent + XMVectorAbs(XMVectorReplicate(BoxExtent.GetZ()) * m2);

		Result.BoxExtent = Vector(NewExtent);
		Result.Origin = Vector(NewOrigin);

		XMVECTOR MaxRadius = m0 * m0;
		MaxRadius = m1 * m1 + MaxRadius;
		MaxRadius = m2 * m2 + MaxRadius;
		MaxRadius = XMVectorMax(XMVectorMax(MaxRadius, XMVectorReplicate(XMVectorGetY(MaxRadius))), XMVectorReplicate(XMVectorGetZ(MaxRadius)));
		Result.SphereRadius = std::sqrt(XMVectorGetX(MaxRadius)) * SphereRadius;

		float const BoxExtentMagnitude = std::sqrt(XMVectorGetX(XMVector3Dot(NewExtent, NewExtent)));
		Result.SphereRadius = std::min(Result.SphereRadius, BoxExtentMagnitude);

		return Result;
	}

}