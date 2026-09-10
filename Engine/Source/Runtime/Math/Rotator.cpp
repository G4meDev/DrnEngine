#include "DrnPCH.h"
#include "Rotator.h"

namespace Drn
{
	const Drn::Rotator Rotator::ZeroRotator(0.0f);
	
	//Rotator::Rotator( const Quat& InQuat )
	//{
	//	*this = InQuat.ToRotator();
	//}

	Quat Rotator::Quaternion() const
	{
		return Quat(Math::DegreesToRadians(Roll), Math::DegreesToRadians(Pitch), Math::DegreesToRadians(Yaw));
	}

}  // namespace Drn