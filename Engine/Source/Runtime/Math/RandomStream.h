#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	struct RandomStream
	{
		RandomStream()
			: InitalSeed(0)
			, Seed(0)
		{}

		RandomStream(int32 InSeed)
		{
			Initalize(InSeed);
		}

		inline void Initalize(int32 InSeed)
		{
			InitalSeed = InSeed;
			Seed = uint32(InSeed);
		}

		inline void Reset() const { Seed = (uint32)InitalSeed; }
		inline int32 GetInitalSeed() const { return InitalSeed; }
		inline void GenerateNewSeed() { Initalize(std::rand()); }

		float GetFraction() const
		{
			MutateSeed();

			float Result;
			*(uint32*)&Result = 0x3F800000U | (Seed >> 9);
			return Result - 1.0f; 
		}

		uint32 GetUnsignedInt() const
		{
			MutateSeed();
			return Seed;
		}

		Vector GetUnitVector() const
		{
			Vector Result;
			float L;

			do
			{
				Result = Vector(GetFraction() * 2.f - 1.f, GetFraction() * 2.f - 1.f, GetFraction() * 2.f - 1.f);
				L = Result.SizeSquared();
			}
			while(L > 1.f || L < KINDA_SMALL_NUMBER);

			return Result.GetUnsafeNormal();
		}

		int32 GetCurrentSeed() const
		{
			return int32(Seed);
		}

		inline int32 RandHelper( int32 A ) const
		{
			return ((A > 0) ? std::trunc(GetFraction() * float(A)) : 0);
		}

		inline int32 RandRange( int32 Min, int32 Max ) const
		{
			const int32 Range = (Max - Min) + 1;

			return Min + RandHelper(Range);
		}

		inline float FRandRange( float InMin, float InMax ) const
		{
			return InMin + (InMax - InMin) * GetFraction();
		}

	private:

		void MutateSeed() const
		{
			Seed = (Seed * 196314165U) + 907633515U; 
		}

		int32 InitalSeed;
		mutable uint32 Seed;
	};
}