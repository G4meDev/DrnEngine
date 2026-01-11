#pragma once

#include "ForwardTypes.h"
#include "Runtime/Math/Math.h"

namespace Drn
{
	class BitArray
	{
		static constexpr uint32 FullWordMask = (uint32)-1;
		static constexpr uint32 BitsPerWord = 32u;
	
	public:

		inline BitArray()
			: NumBits(0)
			, MaxBits(0)
		{}


		inline BitArray(bool bValue, int32 InNumBits)
			: MaxBits(0)
		{
			Init(bValue, InNumBits);
		}

		inline void Init(bool bValue, int32 InNumBits)
		{
			NumBits = InNumBits;
			
			const uint32 NumWords = GetNumWords();
			const uint32 MaxWords = GetMaxWords();

			if (NumWords > 0)
			{
				if (NumWords > MaxWords)
				{
					Allocator.resize(NumWords);
					MaxBits = NumWords * BitsPerWord;
				}

				SetWords(GetData(), NumWords, bValue);
				ClearPartialSlackBits();
			}
		}
	

	private:
		std::vector<uint32> Allocator;
		int32 NumBits;
		int32 MaxBits;

		inline static uint32 CalculateNumWords(int32 NumBits)
		{
			return Math::DivideAndRoundUp(static_cast<uint32>(NumBits), BitsPerWord);
		}

		FORCEINLINE uint32 GetNumWords() const
		{
			return CalculateNumWords(NumBits);
		}

		inline uint32 GetMaxWords() const
		{
			return  CalculateNumWords(MaxBits);
		}

		inline static void SetWords(uint32* Words, int32 NumWords, bool bValue)
		{
			if (NumWords > 8)
			{
				memset(Words, bValue ? 0xff : 0, NumWords * sizeof(uint32));
			}
			else
			{
				uint32 Word = bValue ? ~0u : 0u;
				for (int32 Idx = 0; Idx < NumWords; ++Idx)
				{
					Words[Idx] = Word;
				}
			}
		}

	public:
		inline int32 Num() const { return NumBits; }
		inline int32 Max() const { return MaxBits; }

		inline uint32* GetData() const { return (uint32*)Allocator.data(); }
		inline uint32* GetData() { return Allocator.data(); }

		void ClearPartialSlackBits()
		{
			const int32 UsedBits = NumBits % BitsPerWord;
			if (UsedBits != 0)
			{
				const int32  LastDWORDIndex = NumBits / BitsPerWord;
				const uint32 SlackMask = FullWordMask >> (BitsPerWord - UsedBits);

				uint32* LastDWORD = (GetData() + LastDWORDIndex);
				*LastDWORD = *LastDWORD & SlackMask;
			}
		}

		int32 Add(const bool Value)
		{
			const int32 Index = AddUninitialized(1);
			SetBitNoCheck(Index, Value);
			return Index;
		}

		int32 Add(const bool Value, int32 NumBitsToAdd)
		{
			if (NumBitsToAdd < 0)
			{
				return NumBits;
			}
			const int32 Index = AddUninitialized(NumBitsToAdd);
			SetRange(Index, NumBitsToAdd, Value);
			return Index;
		}

		int32 AddUninitialized(int32 NumBitsToAdd)
		{
			drn_check(NumBitsToAdd >= 0);
			int32 AddedIndex = NumBits;
			if (NumBitsToAdd > 0)
			{
				int32 OldLastWordIndex = NumBits == 0 ? -1 : (NumBits - 1) / BitsPerWord;
				int32 NewLastWordIndex = (NumBits + NumBitsToAdd - 1) / BitsPerWord;
				if (NewLastWordIndex == OldLastWordIndex)
				{
					NumBits += NumBitsToAdd;
				}
				else
				{
					Reserve(NumBits + NumBitsToAdd);
					NumBits += NumBitsToAdd;
					ClearPartialSlackBits();
				}
			}
			return AddedIndex;
		}

		void Reserve(int32 Number)
		{
			if (Number > MaxBits)
			{
				const uint32 MaxDWORDs = CalculateSlackGrow(Number);
				MaxBits = MaxDWORDs * BitsPerWord;
				Realloc();
			}
		}

		int32 CalculateSlackGrow(int32 NumberBits)
		{
			return CalculateNumWords(NumberBits);
		}

		inline void Realloc()
		{
			const uint32 MaxDWORDs = CalculateNumWords(MaxBits);
			Allocator.resize(MaxDWORDs);
			ClearPartialSlackBits();
		}

		void SetBitNoCheck(int32 Index, bool Value)
		{
			uint32& Word = GetData()[Index / BitsPerWord];
			uint32 BitOffset = (Index % BitsPerWord);
			Word = (Word & ~(1 << BitOffset)) | (((uint32)Value) << BitOffset);
		}

		void Empty(int32 ExpectedNumBits = 0)
		{
			ExpectedNumBits = static_cast<int32>(CalculateNumWords(ExpectedNumBits)) * BitsPerWord;
			NumBits = 0;

			MaxBits = std::max(ExpectedNumBits, 4 * (int32)BitsPerWord);
			Realloc();
		}

		void Reset()
		{
			NumBits = 0;
		}

		void SetNumUninitialized(int32 InNumBits)
		{
			//int32 PreviousNumBits = NumBits;
			NumBits = InNumBits;

			if (InNumBits > MaxBits)
			{
				//const int32 PreviousNumDWORDs = CalculateNumWords(PreviousNumBits);
				const uint32 MaxDWORDs = CalculateNumWords(InNumBits);
				Allocator.resize(MaxDWORDs);
				MaxBits = MaxDWORDs * BitsPerWord;
			}

			ClearPartialSlackBits();
		}

		inline void SetRange(int32 Index, int32 NumBitsToSet, bool Value)
		{
			drn_check(Index >= 0 && NumBitsToSet >= 0 && Index + NumBitsToSet <= NumBits);

			if (NumBitsToSet == 0)
			{
				return;
			}

			uint32 StartIndex = Index / BitsPerWord;
			uint32 Count      = (Index + NumBitsToSet + (BitsPerWord - 1)) / BitsPerWord - StartIndex;

			uint32 StartMask  = FullWordMask << (Index % BitsPerWord);
			uint32 EndMask    = FullWordMask >> (BitsPerWord - (Index + NumBitsToSet) % BitsPerWord) % BitsPerWord;

			uint32* Data = GetData() + StartIndex;
			if (Value)
			{
				if (Count == 1)
				{
					*Data |= StartMask & EndMask;
				}
				else
				{
					*Data++ |= StartMask;
					Count -= 2;
					while (Count != 0)
					{
						*Data++ = ~0;
						--Count;
					}
					*Data |= EndMask;
				}
			}
			else
			{
				if (Count == 1)
				{
					*Data &= ~(StartMask & EndMask);
				}
				else
				{
					*Data++ &= ~StartMask;
					Count -= 2;
					while (Count != 0)
					{
						*Data++ = 0;
						--Count;
					}
					*Data &= ~EndMask;
				}
			}
		}

		class BitArrayIterator
		{
		public:
			inline BitArrayIterator(BitArray& InArray,int32 StartIndex = 0)
				: Array(InArray)
				, Index(StartIndex)
				, DWORDIndex(StartIndex >> 5)
				, Mask(1 << (StartIndex & (BitsPerWord - 1)))
			{}

			inline BitArrayIterator& operator++()
			{
				++Index;
				this->Mask <<= 1;
				if(!this->Mask)
				{
					this->Mask = 1;
					++this->DWORDIndex;
				}
				return *this;
			}

			FORCEINLINE explicit operator bool() const
			{ 
				return Index < Array.Num(); 
			}

			FORCEINLINE bool operator !() const 
			{
				return !(bool)*this;
			}

			inline bool GetValue() const 
			{
				uint32 Data = Array.GetData()[this->DWORDIndex];
				return (Data & this->Mask) != 0;
			}
			inline int32 GetIndex() const { return Index; }

		private:
			BitArray& Array;
			int32 Index;
			uint32 Mask;
			int32  DWORDIndex;
		};

		class ConstSetBitIterator
		{
		public:
	
			ConstSetBitIterator(const BitArray& InArray,int32 StartIndex = 0)
				: Array                (InArray)
				, UnvisitedBitMask     ((~0U) << (StartIndex & (BitsPerWord - 1)))
				, CurrentBitIndex      (StartIndex)
				, BaseBitIndex         (StartIndex & ~(BitsPerWord - 1))
				, DWORDIndex(StartIndex >> 5)
				, Mask(1 << (StartIndex & (BitsPerWord - 1)))
			{
				drn_check(StartIndex >= 0 && StartIndex <= Array.Num());
				if (StartIndex != Array.Num())
				{
					FindFirstSetBit();
				}
			}
	
			inline ConstSetBitIterator& operator++()
			{
				UnvisitedBitMask &= ~this->Mask;
				FindFirstSetBit();
	
				return *this;
			}
	
			inline friend bool operator==( const ConstSetBitIterator& Lhs, const ConstSetBitIterator& Rhs ) 
			{
				return Lhs.CurrentBitIndex == Rhs.CurrentBitIndex && &Lhs.Array == &Rhs.Array;
			}
	
			inline friend bool operator!=(const ConstSetBitIterator& Lhs, const ConstSetBitIterator& Rhs)
			{ 
				return !(Lhs == Rhs);
			}
	
			inline explicit operator bool() const
			{ 
				return CurrentBitIndex < Array.Num(); 
			}

			inline bool operator !() const 
			{
				return !(bool)*this;
			}
	
			inline int32 GetIndex() const
			{
				return CurrentBitIndex;
			}
	
		private:
	
			const BitArray& Array;
	
			uint32 UnvisitedBitMask;
			int32 CurrentBitIndex;
			int32 BaseBitIndex;
			uint32 Mask;
			int32  DWORDIndex;
	
			void FindFirstSetBit()
			{
				const uint32* ArrayData      = Array.GetData();
				const int32   ArrayNum       = Array.Num();
				const int32   LastDWORDIndex = (ArrayNum - 1) / BitsPerWord;
	
				uint32 RemainingBitMask = ArrayData[this->DWORDIndex] & UnvisitedBitMask;
				while (!RemainingBitMask)
				{
					++this->DWORDIndex;
					BaseBitIndex += BitsPerWord;
					if (this->DWORDIndex > LastDWORDIndex)
					{
						CurrentBitIndex = ArrayNum;
						return;
					}
	
					RemainingBitMask = ArrayData[this->DWORDIndex];
					UnvisitedBitMask = ~0;
				}
	
				const uint32 NewRemainingBitMask = RemainingBitMask & (RemainingBitMask - 1);
				this->Mask = NewRemainingBitMask ^ RemainingBitMask;
	
				CurrentBitIndex = BaseBitIndex + BitsPerWord - 1 - Math::CountLeadingZeros(this->Mask);

				if (CurrentBitIndex > ArrayNum)
				{
					CurrentBitIndex = ArrayNum;
				}
			}
		};
	};
}