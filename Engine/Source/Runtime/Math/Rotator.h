#pragma once

//#include "Runtime/Math/Quat.h"

namespace Drn
{
	struct Rotator
	{
	public:
		float Pitch;
		float Yaw;
		float Roll;
		
		static const Rotator ZeroRotator;
		
		inline Rotator(float InPitch, float InYaw, float InRoll) : Pitch(InPitch), Yaw(InYaw), Roll(InRoll) {}
		inline Rotator(float InF) : Rotator(InF, InF, InF) {}
		inline Rotator() : Rotator(0.0f) {};
		
		//Rotator(const Quat& InQuat);
		Quat Quaternion() const;
		
		Rotator operator+( const Rotator& R ) const
		{
			return Rotator( Pitch+R.Pitch, Yaw+R.Yaw, Roll+R.Roll );
		}
		
		Rotator operator-( const Rotator& R ) const
		{
			return Rotator( Pitch-R.Pitch, Yaw-R.Yaw, Roll-R.Roll );
		}
		
		Rotator operator*( float Scale ) const
		{
			return Rotator( Pitch*Scale, Yaw*Scale, Roll*Scale );
		}
		
		Rotator operator*=( float Scale )
		{
			Pitch = Pitch*Scale; Yaw = Yaw*Scale; Roll = Roll*Scale;
			return *this;
		}
		
		bool operator==( const Rotator& R ) const
		{
			return Pitch==R.Pitch && Yaw==R.Yaw && Roll==R.Roll;
		}
		
		bool operator!=( const Rotator& V ) const
		{
			return Pitch!=V.Pitch || Yaw!=V.Yaw || Roll!=V.Roll;
		}
		
		Rotator operator+=( const Rotator& R )
		{
			Pitch += R.Pitch; Yaw += R.Yaw; Roll += R.Roll;
			return *this;
		}
		
		Rotator operator-=( const Rotator& R )
		{
			Pitch -= R.Pitch; Yaw -= R.Yaw; Roll -= R.Roll;
			return *this;
		}
		
		inline static float ClampAxis( float Angle )
		{
			Angle = std::fmod(Angle, 360.f);
			if (Angle < 0.f)
			{
				Angle += 360.f;
			}
		
			return Angle;
		}
		
		inline static float NormalizeAxis( float Angle )
		{
			Angle = ClampAxis(Angle);
			if (Angle > 180.f)
			{
				Angle -= 360.f;
			}
		
			return Angle;
		}
		
		inline void Normalize()
		{
			Pitch = NormalizeAxis(Pitch);
			Yaw = NormalizeAxis(Yaw);
			Roll = NormalizeAxis(Roll);
		}
		
		inline Rotator GetNormalized() const
		{
			Rotator Rot = *this;
			Rot.Normalize();
			return Rot;
		}

	private:
		
	};
}