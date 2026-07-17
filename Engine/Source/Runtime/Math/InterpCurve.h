#pragma once

#include "ForwardTypes.h"
#include "Runtime/Math/InterpCurvePoint.h"

#include "Runtime/Math/Math.h"

#if WITH_EDITOR
#include "imgui.h"
#endif

namespace Drn
{
	template<class T>
	class InterpCurve
	{
	public:
		std::vector<InterpCurvePoint<T>> Points;
		bool bIsLooped;
		float LoopKeyOffset;

	public:

		InterpCurve() 
			: bIsLooped(false)
			, LoopKeyOffset(0.f)
		{ 
		}

	public:

		int32 AddPoint( const float InVal, const T &OutVal );
		//int32 MovePoint( int32 PointIndex, float NewInVal );

		//void Reset();
		//void SetLoopKey( float InLoopKey );
		//void ClearLoopKey();

		T Eval( const float InVal, const T& Default = T(ForceInit) ) const;
		//T EvalDerivative( const float InVal, const T& Default = T(ForceInit) ) const;
		//T EvalSecondDerivative( const float InVal, const T& Default = T(ForceInit) ) const;
		//
		//float InaccurateFindNearest( const T &PointInSpace, float& OutDistanceSq ) const;
		//float InaccurateFindNearest( const T &PointInSpace, float& OutDistanceSq, float& OutSegment ) const;
		//float InaccurateFindNearestOnSegment( const T &PointInSpace, int32 PtIdx, float& OutSquaredDistance ) const;
		//
		//void AutoSetTangents(float Tension = 0.0f, bool bStationaryEndpoints = true);
		//
		//void CalcBounds(T& OutMin, T& OutMax, const T& Default = T(ForceInit)) const;

	public:

		friend Archive& operator<<( Archive& Ar, InterpCurve& Curve )
		{
			const int32 PointsCount = Curve.Points.size();
			Ar << PointsCount;
			for (int32 i = 0; i < PointsCount; i++)
			{
				Ar << Curve.Points[i];
			}

			Ar << Curve.bIsLooped;
			Ar << Curve.LoopKeyOffset;

			return Ar;
		}

		friend Archive& operator>>( Archive& Ar, InterpCurve& Curve )
		{
			int32 PointsCount = Curve.Points.size();
			//int32 PointsCount = 0; // @BUG: somehow this does not compile
			Ar >> PointsCount;
			Curve.Points.resize(PointsCount);
			for (int32 i = 0; i < PointsCount; i++)
			{
				Ar >> Curve.Points[i];
			}

			Ar >> Curve.bIsLooped;
			Ar >> Curve.LoopKeyOffset;

			return Ar;
		}

		friend bool operator==(const InterpCurve& Curve1, const InterpCurve& Curve2)
		{
			return (Curve1.Points == Curve2.Points &&
					Curve1.bIsLooped == Curve2.bIsLooped &&
					(!Curve1.bIsLooped || Curve1.LoopKeyOffset == Curve2.LoopKeyOffset));
		}

		friend bool operator!=(const InterpCurve& Curve1, const InterpCurve& Curve2)
		{
			return !(Curve1 == Curve2);
		}

		int32 GetPointIndexForInputValue(const float InValue) const;

#if WITH_EDITOR
		bool Draw(const std::string DisplayName);
#endif
	};

	template< class T > 
	int32 InterpCurve<T>::AddPoint( const float InVal, const T &OutVal )
	{
		int32 i=0; for( i=0; i<Points.size() && Points[i].InVal < InVal; i++);
		Points.insert(Points.begin() + i, InterpCurvePoint< T >(InVal, OutVal));
		return i;
	}


	//template< class T > 
	//int32 InterpCurve<T>::MovePoint( int32 PointIndex, float NewInVal )
	//{
	//	if( PointIndex < 0 || PointIndex >= Points.Num() )
	//		return PointIndex;
	//
	//	const T OutVal = Points[PointIndex].OutVal;
	//	const EInterpCurveMode Mode = Points[PointIndex].InterpMode;
	//	const T ArriveTan = Points[PointIndex].ArriveTangent;
	//	const T LeaveTan = Points[PointIndex].LeaveTangent;
	//
	//	Points.RemoveAt(PointIndex);
	//
	//	const int32 NewPointIndex = AddPoint( NewInVal, OutVal );
	//	Points[NewPointIndex].InterpMode = Mode;
	//	Points[NewPointIndex].ArriveTangent = ArriveTan;
	//	Points[NewPointIndex].LeaveTangent = LeaveTan;
	//
	//	return NewPointIndex;
	//}


	//template< class T > 
	//void InterpCurve<T>::Reset()
	//{
	//	Points.Empty();
	//}
	//
	//
	//template <class T>
	//void InterpCurve<T>::SetLoopKey(float InLoopKey)
	//{
	//	// Can't set a loop key if there are no points
	//	if (Points.Num() == 0)
	//	{
	//		bIsLooped = false;
	//		return;
	//	}
	//
	//	const float LastInKey = Points.Last().InVal;
	//	if (InLoopKey > LastInKey)
	//	{
	//		// Calculate loop key offset from the input key of the final point
	//		bIsLooped = true;
	//		LoopKeyOffset = InLoopKey - LastInKey;
	//	}
	//	else
	//	{
	//		// Specified a loop key lower than the final point; turn off looping.
	//		bIsLooped = false;
	//	}
	//}
	//
	//
	//template <class T>
	//void InterpCurve<T>::ClearLoopKey()
	//{
	//	bIsLooped = false;
	//}


	template< class T >
	int32 InterpCurve<T>::GetPointIndexForInputValue(const float InValue) const
	{
		const int32 NumPoints = Points.size();
		const int32 LastPoint = NumPoints - 1;
	
		drn_check(NumPoints > 0);

		if (InValue < Points[0].InVal)
		{
			return -1;
		}
	
		if (InValue >= Points[LastPoint].InVal)
		{
			return LastPoint;
		}
	
		int32 MinIndex = 0;
		int32 MaxIndex = NumPoints;
	
		while (MaxIndex - MinIndex > 1)
		{
			int32 MidIndex = (MinIndex + MaxIndex) / 2;

			if (Points[MidIndex].InVal <= InValue)
			{
				MinIndex = MidIndex;
			}
			else
			{
				MaxIndex = MidIndex;
			}
		}
	
		return MinIndex;
	}


	template< class T >
	T InterpCurve<T>::Eval(const float InVal, const T& Default) const
	{
		const int32 NumPoints = Points.size();
		const int32 LastPoint = NumPoints - 1;
	
		if (NumPoints == 0)
		{
			return Default;
		}
	
		const int32 Index = GetPointIndexForInputValue(InVal);
	
		// If before the first point, return its value
		if (Index == -1)
		{
			return Points[0].OutVal;
		}
	
		// If on or beyond the last point, return its value.
		if (Index == LastPoint)
		{
			if (!bIsLooped)
			{
				return Points[LastPoint].OutVal;
			}
			else if (InVal >= Points[LastPoint].InVal + LoopKeyOffset)
			{
				// Looped spline: last point is the same as the first point
				return Points[0].OutVal;
			}
		}
	
		// Somewhere within curve range - interpolate.
		drn_check(Index >= 0 && ((bIsLooped && Index < NumPoints) || (!bIsLooped && Index < LastPoint)));
		const bool bLoopSegment = (bIsLooped && Index == LastPoint);
		const int32 NextIndex = bLoopSegment ? 0 : (Index + 1);
	
		const auto& PrevPoint = Points[Index];
		const auto& NextPoint = Points[NextIndex];
	
		const float Diff = bLoopSegment ? LoopKeyOffset : (NextPoint.InVal - PrevPoint.InVal);
	
		if (Diff > 0.0f && PrevPoint.InterpMode != CIM_Constant)
		{
			const float Alpha = (InVal - PrevPoint.InVal) / Diff;
			drn_check(Alpha >= 0.0f && Alpha <= 1.0f);
	
			if (PrevPoint.InterpMode == CIM_Linear)
			{
				return Math::Lerp(PrevPoint.OutVal, NextPoint.OutVal, Alpha);
			}
			else
			{
				return Math::CubicInterp(PrevPoint.OutVal, PrevPoint.LeaveTangent * Diff, NextPoint.OutVal, NextPoint.ArriveTangent * Diff, Alpha);
			}
		}
		else
		{
			return Points[Index].OutVal;
		}
	}


	//template< class T >
	//T InterpCurve<T>::EvalDerivative(const float InVal, const T& Default) const
	//{
	//	const int32 NumPoints = Points.Num();
	//	const int32 LastPoint = NumPoints - 1;
	//
	//	// If no point in curve, return the Default value we passed in.
	//	if (NumPoints == 0)
	//	{
	//		return Default;
	//	}
	//
	//	// Binary search to find index of lower bound of input value
	//	const int32 Index = GetPointIndexForInputValue(InVal);
	//
	//	// If before the first point, return its tangent value
	//	if (Index == -1)
	//	{
	//		return Points[0].LeaveTangent;
	//	}
	//
	//	// If on or beyond the last point, return its tangent value.
	//	if (Index == LastPoint)
	//	{
	//		if (!bIsLooped)
	//		{
	//			return Points[LastPoint].ArriveTangent;
	//		}
	//		else if (InVal >= Points[LastPoint].InVal + LoopKeyOffset)
	//		{
	//			// Looped spline: last point is the same as the first point
	//			return Points[0].ArriveTangent;
	//		}
	//	}
	//
	//	// Somewhere within curve range - interpolate.
	//	check(Index >= 0 && ((bIsLooped && Index < NumPoints) || (!bIsLooped && Index < LastPoint)));
	//	const bool bLoopSegment = (bIsLooped && Index == LastPoint);
	//	const int32 NextIndex = bLoopSegment ? 0 : (Index + 1);
	//
	//	const auto& PrevPoint = Points[Index];
	//	const auto& NextPoint = Points[NextIndex];
	//
	//	const float Diff = bLoopSegment ? LoopKeyOffset : (NextPoint.InVal - PrevPoint.InVal);
	//
	//	if (Diff > 0.0f && PrevPoint.InterpMode != CIM_Constant)
	//	{
	//		if (PrevPoint.InterpMode == CIM_Linear)
	//		{
	//			return (NextPoint.OutVal - PrevPoint.OutVal) / Diff;
	//		}
	//		else
	//		{
	//			const float Alpha = (InVal - PrevPoint.InVal) / Diff;
	//			check(Alpha >= 0.0f && Alpha <= 1.0f);
	//
	//			return FMath::CubicInterpDerivative(PrevPoint.OutVal, PrevPoint.LeaveTangent * Diff, NextPoint.OutVal, NextPoint.ArriveTangent * Diff, Alpha) / Diff;
	//		}
	//	}
	//	else
	//	{
	//		// Derivative of a constant is zero
	//		return T(ForceInit);
	//	}
	//}
	//
	//
	//template< class T >
	//T InterpCurve<T>::EvalSecondDerivative(const float InVal, const T& Default) const
	//{
	//	const int32 NumPoints = Points.Num();
	//	const int32 LastPoint = NumPoints - 1;
	//
	//	// If no point in curve, return the Default value we passed in.
	//	if (NumPoints == 0)
	//	{
	//		return Default;
	//	}
	//
	//	// Binary search to find index of lower bound of input value
	//	const int32 Index = GetPointIndexForInputValue(InVal);
	//
	//	// If before the first point, return 0
	//	if (Index == -1)
	//	{
	//		return T(ForceInit);
	//	}
	//
	//	// If on or beyond the last point, return 0
	//	if (Index == LastPoint)
	//	{
	//		if (!bIsLooped || (InVal >= Points[LastPoint].InVal + LoopKeyOffset))
	//		{
	//			return T(ForceInit);
	//		}
	//	}
	//
	//	// Somewhere within curve range - interpolate.
	//	check(Index >= 0 && ((bIsLooped && Index < NumPoints) || (!bIsLooped && Index < LastPoint)));
	//	const bool bLoopSegment = (bIsLooped && Index == LastPoint);
	//	const int32 NextIndex = bLoopSegment ? 0 : (Index + 1);
	//
	//	const auto& PrevPoint = Points[Index];
	//	const auto& NextPoint = Points[NextIndex];
	//
	//	const float Diff = bLoopSegment ? LoopKeyOffset : (NextPoint.InVal - PrevPoint.InVal);
	//
	//	if (Diff > 0.0f && PrevPoint.InterpMode != CIM_Constant)
	//	{
	//		if (PrevPoint.InterpMode == CIM_Linear)
	//		{
	//			// No change in tangent, return 0.
	//			return T(ForceInit);
	//		}
	//		else
	//		{
	//			const float Alpha = (InVal - PrevPoint.InVal) / Diff;
	//			check(Alpha >= 0.0f && Alpha <= 1.0f);
	//
	//			return FMath::CubicInterpSecondDerivative(PrevPoint.OutVal, PrevPoint.LeaveTangent * Diff, NextPoint.OutVal, NextPoint.ArriveTangent * Diff, Alpha) / (Diff * Diff);
	//		}
	//	}
	//	else
	//	{
	//		// Second derivative of a constant is zero
	//		return T(ForceInit);
	//	}
	//}
	//
	//
	//template< class T >
	//float InterpCurve<T>::InaccurateFindNearest(const T &PointInSpace, float& OutDistanceSq) const
	//{
	//	float OutSegment;
	//	return InaccurateFindNearest(PointInSpace, OutDistanceSq, OutSegment);
	//}
	//
	//template< class T >
	//float InterpCurve<T>::InaccurateFindNearest(const T &PointInSpace, float& OutDistanceSq, float& OutSegment) const
	//{
	//	const int32 NumPoints = Points.Num();
	//	const int32 NumSegments = bIsLooped ? NumPoints : NumPoints - 1;
	//
	//	if (NumPoints > 1)
	//	{
	//		float BestDistanceSq;
	//		float BestResult = InaccurateFindNearestOnSegment(PointInSpace, 0, BestDistanceSq);
	//		float BestSegment = 0;
	//		for (int32 Segment = 1; Segment < NumSegments; ++Segment)
	//		{
	//			float LocalDistanceSq;
	//			float LocalResult = InaccurateFindNearestOnSegment(PointInSpace, Segment, LocalDistanceSq);
	//			if (LocalDistanceSq < BestDistanceSq)
	//			{
	//				BestDistanceSq = LocalDistanceSq;
	//				BestResult = LocalResult;
	//				BestSegment = (float)Segment;
	//			}
	//		}
	//		OutDistanceSq = BestDistanceSq;
	//		OutSegment = BestSegment;
	//		return BestResult;
	//	}
	//
	//	if (NumPoints == 1)
	//	{
	//		OutDistanceSq = (PointInSpace - Points[0].OutVal).SizeSquared();
	//		OutSegment = 0;
	//		return Points[0].InVal;
	//	}
	//
	//	return 0.0f;
	//}
	//
	//
	//template< class T >
	//float InterpCurve<T>::InaccurateFindNearestOnSegment(const T& PointInSpace, int32 PtIdx, float& OutSquaredDistance) const
	//{
	//	const int32 NumPoints = Points.Num();
	//	const int32 LastPoint = NumPoints - 1;
	//	const int32 NextPtIdx = (bIsLooped && PtIdx == LastPoint) ? 0 : (PtIdx + 1);
	//	check(PtIdx >= 0 && ((bIsLooped && PtIdx < NumPoints) || (!bIsLooped && PtIdx < LastPoint)));
	//
	//	const float NextInVal = (bIsLooped && PtIdx == LastPoint) ? (Points[LastPoint].InVal + LoopKeyOffset) : Points[NextPtIdx].InVal;
	//
	//	if (CIM_Constant == Points[PtIdx].InterpMode)
	//	{
	//		const float Distance1 = (Points[PtIdx].OutVal - PointInSpace).SizeSquared();
	//		const float Distance2 = (Points[NextPtIdx].OutVal - PointInSpace).SizeSquared();
	//		if (Distance1 < Distance2)
	//		{
	//			OutSquaredDistance = Distance1;
	//			return Points[PtIdx].InVal;
	//		}
	//		OutSquaredDistance = Distance2;
	//		return NextInVal;
	//	}
	//
	//	const float Diff = NextInVal - Points[PtIdx].InVal;
	//	if (CIM_Linear == Points[PtIdx].InterpMode)
	//	{
	//		// like in function: FMath::ClosestPointOnLine
	//		const float A = (Points[PtIdx].OutVal - PointInSpace) | (Points[NextPtIdx].OutVal - Points[PtIdx].OutVal);
	//		const float B = (Points[NextPtIdx].OutVal - Points[PtIdx].OutVal).SizeSquared();
	//		const float V = FMath::Clamp(-A / B, 0.f, 1.f);
	//		OutSquaredDistance = (FMath::Lerp(Points[PtIdx].OutVal, Points[NextPtIdx].OutVal, V) - PointInSpace).SizeSquared();
	//		return V * Diff + Points[PtIdx].InVal;
	//	}
	//
	//	{
	//		const int32 PointsChecked = 3;
	//		const int32 IterationNum = 3;
	//		const float Scale = 0.75;
	//
	//		// Newton's methods is repeated 3 times, starting with t = 0, 0.5, 1.
	//		float ValuesT[PointsChecked];
	//		ValuesT[0] = 0.0f;
	//		ValuesT[1] = 0.5f;
	//		ValuesT[2] = 1.0f;
	//
	//		T InitialPoints[PointsChecked];
	//		InitialPoints[0] = Points[PtIdx].OutVal;
	//		InitialPoints[1] = FMath::CubicInterp(Points[PtIdx].OutVal, Points[PtIdx].LeaveTangent * Diff, Points[NextPtIdx].OutVal, Points[NextPtIdx].ArriveTangent * Diff, ValuesT[1]);
	//		InitialPoints[2] = Points[NextPtIdx].OutVal;
	//
	//		float DistancesSq[PointsChecked];
	//
	//		for (int32 point = 0; point < PointsChecked; ++point)
	//		{
	//			//Algorithm explanation: http://permalink.gmane.org/gmane.games.devel.sweng/8285
	//			T FoundPoint = InitialPoints[point];
	//			float LastMove = 1.0f;
	//			for (int32 iter = 0; iter < IterationNum; ++iter)
	//			{
	//				const T LastBestTangent = FMath::CubicInterpDerivative(Points[PtIdx].OutVal, Points[PtIdx].LeaveTangent * Diff, Points[NextPtIdx].OutVal, Points[NextPtIdx].ArriveTangent * Diff, ValuesT[point]);
	//				const T Delta = (PointInSpace - FoundPoint);
	//				float Move = (LastBestTangent | Delta) / LastBestTangent.SizeSquared();
	//				Move = FMath::Clamp(Move, -LastMove*Scale, LastMove*Scale);
	//				ValuesT[point] += Move;
	//				ValuesT[point] = FMath::Clamp(ValuesT[point], 0.0f, 1.0f);
	//				LastMove = FMath::Abs(Move);
	//				FoundPoint = FMath::CubicInterp(Points[PtIdx].OutVal, Points[PtIdx].LeaveTangent * Diff, Points[NextPtIdx].OutVal, Points[NextPtIdx].ArriveTangent * Diff, ValuesT[point]);
	//			}
	//			DistancesSq[point] = (FoundPoint - PointInSpace).SizeSquared();
	//			ValuesT[point] = ValuesT[point] * Diff + Points[PtIdx].InVal;
	//		}
	//
	//		if (DistancesSq[0] <= DistancesSq[1] && DistancesSq[0] <= DistancesSq[2])
	//		{
	//			OutSquaredDistance = DistancesSq[0];
	//			return ValuesT[0];
	//		}
	//		if (DistancesSq[1] <= DistancesSq[2])
	//		{
	//			OutSquaredDistance = DistancesSq[1];
	//			return ValuesT[1];
	//		}
	//		OutSquaredDistance = DistancesSq[2];
	//		return ValuesT[2];
	//	}
	//}
	//
	//
	//template< class T >
	//void InterpCurve<T>::AutoSetTangents(float Tension, bool bStationaryEndpoints)
	//{
	//	const int32 NumPoints = Points.Num();
	//	const int32 LastPoint = NumPoints - 1;
	//
	//	// Iterate over all points in this InterpCurve
	//	for (int32 PointIndex = 0; PointIndex < NumPoints; PointIndex++)
	//	{
	//		const int32 PrevIndex = (PointIndex == 0) ? (bIsLooped ? LastPoint : 0) : (PointIndex - 1);
	//		const int32 NextIndex = (PointIndex == LastPoint) ? (bIsLooped ? 0 : LastPoint) : (PointIndex + 1);
	//
	//		auto& ThisPoint = Points[PointIndex];
	//		const auto& PrevPoint = Points[PrevIndex];
	//		const auto& NextPoint = Points[NextIndex];
	//
	//		if (ThisPoint.InterpMode == CIM_CurveAuto || ThisPoint.InterpMode == CIM_CurveAutoClamped)
	//		{
	//			if (bStationaryEndpoints && (PointIndex == 0 || (PointIndex == LastPoint && !bIsLooped)))
	//			{
	//				// Start and end points get zero tangents if bStationaryEndpoints is true
	//				ThisPoint.ArriveTangent = T(ForceInit);
	//				ThisPoint.LeaveTangent = T(ForceInit);
	//			}
	//			else if (PrevPoint.IsCurveKey())
	//			{
	//				const bool bWantClamping = (ThisPoint.InterpMode == CIM_CurveAutoClamped);
	//				T Tangent;
	//
	//				const float PrevTime = (bIsLooped && PointIndex == 0) ? (ThisPoint.InVal - LoopKeyOffset) : PrevPoint.InVal;
	//				const float NextTime = (bIsLooped && PointIndex == LastPoint) ? (ThisPoint.InVal + LoopKeyOffset) : NextPoint.InVal;
	//
	//				ComputeCurveTangent(
	//					PrevTime,			// Previous time
	//					PrevPoint.OutVal,	// Previous point
	//					ThisPoint.InVal,	// Current time
	//					ThisPoint.OutVal,	// Current point
	//					NextTime,			// Next time
	//					NextPoint.OutVal,	// Next point
	//					Tension,			// Tension
	//					bWantClamping,		// Want clamping?
	//					Tangent);			// Out
	//
	//				ThisPoint.ArriveTangent = Tangent;
	//				ThisPoint.LeaveTangent = Tangent;
	//			}
	//			else
	//			{
	//				// Following on from a line or constant; set curve tangent equal to that so there are no discontinuities
	//				ThisPoint.ArriveTangent = PrevPoint.ArriveTangent;
	//				ThisPoint.LeaveTangent = PrevPoint.LeaveTangent;
	//			}
	//		}
	//		else if (ThisPoint.InterpMode == CIM_Linear)
	//		{
	//			T Tangent = NextPoint.OutVal - ThisPoint.OutVal;
	//			ThisPoint.ArriveTangent = Tangent;
	//			ThisPoint.LeaveTangent = Tangent;
	//		}
	//		else if (ThisPoint.InterpMode == CIM_Constant)
	//		{
	//			ThisPoint.ArriveTangent = T(ForceInit);
	//			ThisPoint.LeaveTangent = T(ForceInit);
	//		}
	//	}
	//}
	//
	//
	//template< class T >
	//void InterpCurve<T>::CalcBounds(T& OutMin, T& OutMax, const T& Default) const
	//{
	//	const int32 NumPoints = Points.Num();
	//
	//	if (NumPoints == 0)
	//	{
	//		OutMin = Default;
	//		OutMax = Default;
	//	}
	//	else if (NumPoints == 1)
	//	{
	//		OutMin = Points[0].OutVal;
	//		OutMax = Points[0].OutVal;
	//	}
	//	else
	//	{
	//		OutMin = Points[0].OutVal;
	//		OutMax = Points[0].OutVal;
	//
	//		const int32 NumSegments = bIsLooped ? NumPoints : (NumPoints - 1);
	//
	//		for (int32 Index = 0; Index < NumSegments; Index++)
	//		{
	//			const int32 NextIndex = (Index == NumPoints - 1) ? 0 : (Index + 1);
	//			CurveFindIntervalBounds(Points[Index], Points[NextIndex], OutMin, OutMax, 0.0f);
	//		}
	//	}
	//}


	typedef InterpCurve<float> InterpCurveFloat;
	//typedef InterpCurve<Vector2> InterpCurveVector2;
	typedef InterpCurve<Vector> InterpCurveVector;
	//typedef InterpCurve<Quat> InterpCurveQuat;
}