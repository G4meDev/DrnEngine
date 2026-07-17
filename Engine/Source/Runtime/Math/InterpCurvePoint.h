#pragma once

//#include "ForwardTypes.h"

namespace Drn
{
	enum EInterpCurveMode
	{
		CIM_Linear,
		CIM_CurveAuto,
		CIM_Constant,
		CIM_CurveUser,
		CIM_CurveBreak,
		CIM_CurveAutoClamped,
		CIM_Unknown
	};

	template< class T >
	class InterpCurvePoint
	{
	public:

		float InVal;

		T OutVal;
		T ArriveTangent; 
		T LeaveTangent; 

		EInterpCurveMode InterpMode; 

	public:

		InterpCurvePoint() { };
		InterpCurvePoint( const float In, const T &Out );
		InterpCurvePoint( const float In, const T &Out, const T &InArriveTangent, const T &InLeaveTangent, const EInterpCurveMode InInterpMode );

	public:

		bool IsCurveKey() const;

	public:

		friend Archive& operator<<( Archive& Ar, InterpCurvePoint& Point )
		{
			Ar << Point.InVal << Point.OutVal;
			Ar << Point.ArriveTangent << Point.LeaveTangent;
			Ar << (uint32)Point.InterpMode;
			return Ar;
		}

		friend Archive& operator>>( Archive& Ar, InterpCurvePoint& Point )
		{
			Ar >> Point.InVal >> Point.OutVal;
			Ar >> Point.ArriveTangent >> Point.LeaveTangent;
			Ar >> *(uint32*)&Point.InterpMode;
			return Ar;
		}

		friend bool operator==( const InterpCurvePoint& Point1, const InterpCurvePoint& Point2 )
		{
			return (Point1.InVal == Point2.InVal &&
					Point1.OutVal == Point2.OutVal &&
					Point1.ArriveTangent == Point2.ArriveTangent &&
					Point1.LeaveTangent == Point2.LeaveTangent &&
					Point1.InterpMode == Point2.InterpMode);
		}

		friend bool operator!=(const InterpCurvePoint& Point1, const InterpCurvePoint& Point2)
		{
			return !(Point1 == Point2);
		}

#if WITH_EDITOR
		bool Draw(int32 Index);
#endif
	};


	template< class T > 
	InterpCurvePoint<T>::InterpCurvePoint( const float In, const T &Out )
		: InVal(In)
		, OutVal(Out)
	{
		memset( &ArriveTangent, 0, sizeof(T) );	
		memset( &LeaveTangent, 0, sizeof(T) );
	
		InterpMode = CIM_Linear;
	}
	
	
	template< class T > 
	InterpCurvePoint<T>::InterpCurvePoint( const float In, const T &Out, const T &InArriveTangent, const T &InLeaveTangent, const EInterpCurveMode InInterpMode)
		: InVal(In)
		, OutVal(Out)
		, ArriveTangent(InArriveTangent)
		, LeaveTangent(InLeaveTangent)
		, InterpMode(InInterpMode)
	{ }
	
	
	template< class T > 
	bool InterpCurvePoint<T>::IsCurveKey() const
	{
		return ((InterpMode == CIM_CurveAuto) || (InterpMode == CIM_CurveAutoClamped) || (InterpMode == CIM_CurveUser) || (InterpMode == CIM_CurveBreak));
	}
	
	///**
	// * Clamps a tangent formed by the specified control point values
	// */
	//CORE_API float ClampFloatTangent( float PrevPointVal, float PrevTime, float CurPointVal, float CurTime, float NextPointVal, float NextTime );
	//
	//
	///** Computes Tangent for a curve segment */
	//template< class T, class U > 
	//inline void AutoCalcTangent( const T& PrevP, const T& P, const T& NextP, const U& Tension, T& OutTan )
	//{
	//	OutTan = (1.f - Tension) * ( (P - PrevP) + (NextP - P) );
	//}
	//
	//
	///**
	// * This actually returns the control point not a tangent. This is expected by the CubicInterp function for Quaternions
	// */
	//template< class U > 
	//inline void AutoCalcTangent( const FQuat& PrevP, const FQuat& P, const FQuat& NextP, const U& Tension, FQuat& OutTan  )
	//{
	//	FQuat::CalcTangents(PrevP, P, NextP, Tension, OutTan);
	//}
	//
	//
	///** Computes a tangent for the specified control point.  General case, doesn't support clamping. */
	//template< class T >
	//inline void ComputeCurveTangent( float PrevTime, const T& PrevPoint,
	//							float CurTime, const T& CurPoint,
	//							float NextTime, const T& NextPoint,
	//							float Tension,
	//							bool bWantClamping,
	//							T& OutTangent )
	//{
	//	// NOTE: Clamping not supported for non-float vector types (bWantClamping is ignored)
	//
	//	AutoCalcTangent( PrevPoint, CurPoint, NextPoint, Tension, OutTangent );
	//
	//	const float PrevToNextTimeDiff = FMath::Max< float >( KINDA_SMALL_NUMBER, NextTime - PrevTime );
	//
	//	OutTangent /= PrevToNextTimeDiff;
	//}
	//
	//
	///**
	// * Computes a tangent for the specified control point; supports clamping, but only works
	// * with floats or contiguous arrays of floats.
	// */
	//template< class T >
	//inline void ComputeClampableFloatVectorCurveTangent( float PrevTime, const T& PrevPoint,
	//												float CurTime, const T& CurPoint,
	//												float NextTime, const T& NextPoint,
	//												float Tension,
	//												bool bWantClamping,
	//												T& OutTangent )
	//{
	//	// Clamp the tangents if we need to do that
	//	if( bWantClamping )
	//	{
	//		// NOTE: We always treat the type as an array of floats
	//		float* PrevPointVal = ( float* )&PrevPoint;
	//		float* CurPointVal = ( float* )&CurPoint;
	//		float* NextPointVal = ( float* )&NextPoint;
	//		float* OutTangentVal = ( float* )&OutTangent;
	//		for( int32 CurValPos = 0; CurValPos < sizeof( T ); CurValPos += sizeof( float ) )
	//		{
	//			// Clamp it!
	//			const float ClampedTangent =
	//				ClampFloatTangent(
	//					*PrevPointVal, PrevTime,
	//					*CurPointVal, CurTime,
	//					*NextPointVal, NextTime );
	//
	//			// Apply tension value
	//			*OutTangentVal = ( 1.0f - Tension ) * ClampedTangent;
	//
	//
	//			// Advance pointers
	//			++OutTangentVal;
	//			++PrevPointVal;
	//			++CurPointVal;
	//			++NextPointVal;
	//		}
	//	}
	//	else
	//	{
	//		// No clamping needed
	//		AutoCalcTangent( PrevPoint, CurPoint, NextPoint, Tension, OutTangent );
	//
	//		const float PrevToNextTimeDiff = FMath::Max< float >( KINDA_SMALL_NUMBER, NextTime - PrevTime );
	//
	//		OutTangent /= PrevToNextTimeDiff;
	//	}
	//}
	//
	//
	///** Computes a tangent for the specified control point.  Special case for float types; supports clamping. */
	//inline void ComputeCurveTangent( float PrevTime, const float& PrevPoint,
	//									float CurTime, const float& CurPoint,
	//									float NextTime, const float& NextPoint,
	//									float Tension,
	//									bool bWantClamping,
	//									float& OutTangent )
	//{
	//	ComputeClampableFloatVectorCurveTangent(
	//		PrevTime, PrevPoint,
	//		CurTime, CurPoint,
	//		NextTime, NextPoint,
	//		Tension, bWantClamping, OutTangent );
	//}
	//
	//
	///** Computes a tangent for the specified control point.  Special case for FVector types; supports clamping. */
	//inline void ComputeCurveTangent( float PrevTime, const FVector& PrevPoint,
	//									float CurTime, const FVector& CurPoint,
	//									float NextTime, const FVector& NextPoint,
	//									float Tension,
	//									bool bWantClamping,
	//									FVector& OutTangent )
	//{
	//	ComputeClampableFloatVectorCurveTangent(
	//		PrevTime, PrevPoint,
	//		CurTime, CurPoint,
	//		NextTime, NextPoint,
	//		Tension, bWantClamping, OutTangent );
	//}
	//
	//
	///** Computes a tangent for the specified control point.  Special case for FVector2D types; supports clamping. */
	//inline void ComputeCurveTangent( float PrevTime, const FVector2D& PrevPoint,
	//									float CurTime, const FVector2D& CurPoint,
	//									float NextTime, const FVector2D& NextPoint,
	//									float Tension,
	//									bool bWantClamping,
	//									FVector2D& OutTangent )
	//{
	//	ComputeClampableFloatVectorCurveTangent(
	//		PrevTime, PrevPoint,
	//		CurTime, CurPoint,
	//		NextTime, NextPoint,
	//		Tension, bWantClamping, OutTangent );
	//}
	//
	//
	///** Computes a tangent for the specified control point.  Special case for FTwoVectors types; supports clamping. */
	//inline void ComputeCurveTangent( float PrevTime, const FTwoVectors& PrevPoint,
	//									float CurTime, const FTwoVectors& CurPoint,
	//									float NextTime, const FTwoVectors& NextPoint,
	//									float Tension,
	//									bool bWantClamping,
	//									FTwoVectors& OutTangent )
	//{
	//	ComputeClampableFloatVectorCurveTangent(
	//		PrevTime, PrevPoint,
	//		CurTime, CurPoint,
	//		NextTime, NextPoint,
	//		Tension, bWantClamping, OutTangent );
	//}
	//
	///**
	// * Calculate bounds of float intervals
	// *
	// * @param Start interp curve point at Start
	// * @param End interp curve point at End
	// * @param CurrentMin Input and Output could be updated if needs new interval minimum bound
	// * @param CurrentMax  Input and Output could be updated if needs new interval maximmum bound
	// */
	//void CORE_API CurveFloatFindIntervalBounds( const InterpCurvePoint<float>& Start, const InterpCurvePoint<float>& End, float& CurrentMin, float& CurrentMax );
	//
	//
	///**
	// * Calculate bounds of 2D vector intervals
	// *
	// * @param Start interp curve point at Start
	// * @param End interp curve point at End
	// * @param CurrentMin Input and Output could be updated if needs new interval minimum bound
	// * @param CurrentMax  Input and Output could be updated if needs new interval maximmum bound
	// */
	//void CORE_API CurveVector2DFindIntervalBounds( const InterpCurvePoint<FVector2D>& Start, const InterpCurvePoint<FVector2D>& End, FVector2D& CurrentMin, FVector2D& CurrentMax );
	//
	//
	///**
	// * Calculate bounds of vector intervals
	// *
	// * @param Start interp curve point at Start
	// * @param End interp curve point at End
	// * @param CurrentMin Input and Output could be updated if needs new interval minimum bound
	// * @param CurrentMax  Input and Output could be updated if needs new interval maximmum bound
	// */
	//void CORE_API CurveVectorFindIntervalBounds( const InterpCurvePoint<FVector>& Start, const InterpCurvePoint<FVector>& End, FVector& CurrentMin, FVector& CurrentMax );
	//
	//
	///**
	// * Calculate bounds of twovector intervals
	// *
	// * @param Start interp curve point at Start
	// * @param End interp curve point at End
	// * @param CurrentMin Input and Output could be updated if needs new interval minimum bound
	// * @param CurrentMax  Input and Output could be updated if needs new interval maximmum bound
	// */
	//void CORE_API CurveTwoVectorsFindIntervalBounds(const InterpCurvePoint<FTwoVectors>& Start, const InterpCurvePoint<FTwoVectors>& End, FTwoVectors& CurrentMin, FTwoVectors& CurrentMax);
	//
	//
	///**
	// * Calculate bounds of color intervals
	// *
	// * @param Start interp curve point at Start
	// * @param End interp curve point at End
	// * @param CurrentMin Input and Output could be updated if needs new interval minimum bound
	// * @param CurrentMax  Input and Output could be updated if needs new interval maximmum bound
	// */
	//void CORE_API CurveLinearColorFindIntervalBounds( const InterpCurvePoint<FLinearColor>& Start, const InterpCurvePoint<FLinearColor>& End, FLinearColor& CurrentMin, FLinearColor& CurrentMax );
	//
	//
	//template< class T, class U > 
	//inline void CurveFindIntervalBounds( const InterpCurvePoint<T>& Start, const InterpCurvePoint<T>& End, T& CurrentMin, T& CurrentMax, const U& Dummy )
	//{ }
	//
	//
	//template< class U > 
	//inline void CurveFindIntervalBounds( const InterpCurvePoint<float>& Start, const InterpCurvePoint<float>& End, float& CurrentMin, float& CurrentMax, const U& Dummy )
	//{
	//	CurveFloatFindIntervalBounds(Start, End, CurrentMin, CurrentMax);
	//}
	//
	//
	//template< class U > 
	//void CurveFindIntervalBounds( const InterpCurvePoint<FVector2D>& Start, const InterpCurvePoint<FVector2D>& End, FVector2D& CurrentMin, FVector2D& CurrentMax, const U& Dummy )
	//{
	//	CurveVector2DFindIntervalBounds(Start, End, CurrentMin, CurrentMax);
	//}
	//
	//
	//template< class U > 
	//inline void CurveFindIntervalBounds( const InterpCurvePoint<FVector>& Start, const InterpCurvePoint<FVector>& End, FVector& CurrentMin, FVector& CurrentMax, const U& Dummy )
	//{
	//	CurveVectorFindIntervalBounds(Start, End, CurrentMin, CurrentMax);
	//}
	//
	//
	//template< class U > 
	//inline void CurveFindIntervalBounds( const InterpCurvePoint<FTwoVectors>& Start, const InterpCurvePoint<FTwoVectors>& End, FTwoVectors& CurrentMin, FTwoVectors& CurrentMax, const U& Dummy )
	//{
	//	CurveTwoVectorsFindIntervalBounds(Start, End, CurrentMin, CurrentMax);
	//}
	//
	//
	//template< class U > 
	//inline void CurveFindIntervalBounds( const InterpCurvePoint<FLinearColor>& Start, const InterpCurvePoint<FLinearColor>& End, FLinearColor& CurrentMin, FLinearColor& CurrentMax, const U& Dummy )
	//{
	//	CurveLinearColorFindIntervalBounds(Start, End, CurrentMin, CurrentMax);
	//}


	typedef InterpCurvePoint<float> InterpCurvePointFloat;
	typedef InterpCurvePoint<Vector2> InterpCurvePointVector2;
	typedef InterpCurvePoint<Vector> InterpCurvePointVector;
	typedef InterpCurvePoint<Quat> InterpCurvePointQuat;
}