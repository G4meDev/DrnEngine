#include "DrnPCH.h"
#include "Sphere.h"

namespace Drn
{
	Sphere& Sphere::operator+=( const Sphere& Other )
	{
		if (Radius == 0.f)
		{
			*this = Other;
		}
		else if (IsInside(Other))
		{
			*this = Other;
		}
		else if (Other.IsInside(*this))
		{
			// no change		
		}
		else
		{
			Sphere NewSphere;

			Vector DirToOther = Other.Center - Center;
			Vector UnitDirToOther = DirToOther;
			UnitDirToOther.GetSafeNormal();

			float NewRadius = (DirToOther.Length() + Other.Radius + Radius) * 0.5f;

			Vector End1 = Other.Center + UnitDirToOther*Other.Radius;
			Vector End2 = Center - UnitDirToOther*Radius;
			Vector NewCenter = (End1 + End2)*0.5f;

			NewSphere.Center = NewCenter; 
			NewSphere.Radius = NewRadius;

			*this = NewSphere;
		}

		return *this;
	}

	bool Sphere::IsInside( const Sphere& Other, float Tolerance ) const
	{
		if (Radius > Other.Radius + Tolerance)
		{
			return false;
		}

		return (Center - Other.Center).SizeSquared() <= Math::Square(Other.Radius + Tolerance - Radius);
	}

}