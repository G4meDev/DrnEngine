#pragma once

#include "string"
#include "map"

#include "Runtime/Misc/DebugHelper.h"
#include "Runtime/Core/CityHash.h"
#include "Runtime/Core/ApplicationMisc.h"

#define STRINGID_SIZE 1024

namespace Drn
{
	static constexpr uint32 StringIdMaxBlockBits = 13;
	static constexpr uint32 StringIdBlockOffsetBits = 16;
	static constexpr uint32 StringIdMaxBlocks = 1 << StringIdMaxBlockBits;
	static constexpr uint32 StringIdBlockOffsets = 1 << StringIdBlockOffsetBits;

	struct StringIdView
	{
		StringIdView() : Str(nullptr), Len(0) {}
		StringIdView(const char* InStr, uint32 InLen) : Str(InStr), Len(InLen) {}

		const char* Str;
		uint32 Len;

		int32 BytesWithTerminator() const
		{
			return (Len + 1) * sizeof(char);
		}

		int32 BytesWithoutTerminator() const
		{
			return Len * sizeof(char);
		}
	};

	struct StringIdEntry
	{
	private:

		uint16 Length;
		char Str[STRINGID_SIZE];

		StringIdEntry(const StringIdEntry&)				= delete;
		StringIdEntry(StringIdEntry&&)					= delete;
		StringIdEntry& operator=(const StringIdEntry&)	= delete;
		StringIdEntry& operator=(StringIdEntry&&)		= delete;

		friend class StringIdEntryAllocator;
		friend class StringIdEntryHandle;
		friend class StringID;

	public:
		inline int32 GetLength() const
		{
			return Length;
		}

		inline std::string GetStr() const
		{
			return std::string(Str, Str + Length);
		}

		inline StringIdView MakeView() const
		{
			return StringIdView(Str, Length);
		}

		inline void Store(const char* InStr, int32 Size)
		{
			drn_check(Size < STRINGID_SIZE);

			memcpy(Str, InStr, sizeof(char) * Size);
			Length = Size;
		}

		inline static int32 GetDataOffset() { return offsetof(StringIdEntry, Str) ; }

		inline static int32 GetSize(int32 Length) { return Align(GetDataOffset() + Length * sizeof(char), alignof(StringIdEntry)); };
		inline static int32 GetSize(const char* Name) { return GetSize(strlen(Name)); };
	};

	struct StringIdEntryHandle
	{
		StringIdEntryHandle()
			: Block(0)
			, Offset(0)
		{}

		StringIdEntryHandle(uint32 InBlock, uint32 InOffset)
			: Block(InBlock)
			, Offset(InOffset)
		{}

		inline bool operator==( const StringIdEntryHandle& Other ) const { return Data == Other.Data; }
		inline bool operator!=( const StringIdEntryHandle& Other ) const { return Data != Other.Data; }

		union
		{
			struct
			{
				uint32 Block;
				uint32 Offset;
			};
			uint64 Data;
		};
	};

	class StringIdEntryAllocator
	{
	public:
		static constexpr uint32 Stride = alignof(StringIdEntry);
		static constexpr uint32 BlockSizeBytes = Stride * StringIdBlockOffsets;

		StringIdEntryAllocator()
		{
			Blocks[0] = AllocBlock();
		}

		~StringIdEntryAllocator()
		{
			for (uint32 Index = 0; Index <= CurrentBlock; Index++)
			{
				free(Blocks[Index]);
			}
		}

		void ReserveBlocks(uint32 Num)
		{
			for (uint32 Idx = Num - 1; Idx > CurrentBlock && Blocks[Idx] == nullptr; --Idx)
			{
				Blocks[Idx] = AllocBlock();
			}
		}

		StringIdEntryHandle Allocate(uint32 Bytes)
		{
			Bytes = Align(Bytes, Stride);
			drn_check(Bytes <= BlockSizeBytes);

			if (BlockSizeBytes - CurrentByteCursor < Bytes)
			{
				AllocateNewBlock();
			}

			uint32 ByteOffset = CurrentByteCursor;
			CurrentByteCursor += Bytes;

			drn_check(ByteOffset % Stride == 0 && ByteOffset / Stride < StringIdBlockOffsets);

			return StringIdEntryHandle(CurrentBlock, ByteOffset / Stride);
		}

		StringIdEntryHandle Create(StringIdView Name)
		{
			ApplicationMisc::Prefetch(Blocks[CurrentBlock]);
			StringIdEntryHandle Handle = Allocate(StringIdEntry::GetDataOffset() + Name.BytesWithoutTerminator());
			StringIdEntry& Entry = Resolve(Handle);

			Entry.Store(Name.Str, Name.Len);
			return Handle;
		}

		StringIdEntry& Resolve(StringIdEntryHandle Handle) const
		{
			return *reinterpret_cast<StringIdEntry*>(Blocks[Handle.Block] + Stride * Handle.Offset);
		}

		uint32 NumBlocks() const { return CurrentBlock + 1; }

		void DebugDump(std::vector<const StringIdEntry*>& Out) const
		{
			for (uint32 BlockIdx = 0; BlockIdx < CurrentBlock; ++BlockIdx)
			{
				DebugDumpBlock(Blocks[BlockIdx], BlockSizeBytes, Out);
			}

			DebugDumpBlock(Blocks[CurrentBlock], CurrentByteCursor, Out);
		}

	private:
		static void DebugDumpBlock(const uint8* It, uint32 BlockSize, std::vector<const StringIdEntry*>& Out)
		{
			const uint8* End = It + BlockSize - StringIdEntry::GetDataOffset();
			while (It < End)
			{
				const StringIdEntry* Entry = (const StringIdEntry*)It;
				if (uint32 Len = Entry->Length)
				{
					Out.push_back(Entry);
					It += StringIdEntry::GetSize(Len);
				}
				else // Null-terminator entry found
				{
					break;
				}
			}
		}

		static uint8* AllocBlock()
		{
			return (uint8*)malloc(BlockSizeBytes);
		}

		void AllocateNewBlock()
		{
			if (CurrentByteCursor + StringIdEntry::GetDataOffset() <= BlockSizeBytes)
			{
				StringIdEntry* Terminator = (StringIdEntry*)(Blocks[CurrentBlock] + CurrentByteCursor);
				Terminator->Length = 0;
			}

			++CurrentBlock;
			CurrentByteCursor = 0;

			//checkf(CurrentBlock < FNameMaxBlocks, TEXT("FName overflow, allocated %dMB of string data. "
			//	"FName strings are never freed and should be created sparingly. "
			//	"Some system might be generating too many FNames, see call stack. "), FNameMaxBlocks * BlockSizeBytes >> 20);

			// Allocate block unless it's already reserved
			if (Blocks[CurrentBlock] == nullptr)
			{
				Blocks[CurrentBlock] = AllocBlock();
			}

			ApplicationMisc::Prefetch(Blocks[CurrentBlock]);
		}

		uint32 CurrentBlock = 0;
		uint32 CurrentByteCursor = 0;
		uint8* Blocks[StringIdMaxBlocks] = {};
	};

	struct StringIdHash
	{
		StringIdHash() {}

		StringIdHash(const StringIdView& InView)
			: View(InView)
		{
			Hash = CityHash64(InView.Str, InView.Len);
		}

		StringIdHash(uint64 InHash, const StringIdView& InView)
			: View(InView)
			, Hash(InHash)
		{}

		bool operator==(const StringIdHash& Other) const
		{
			return (View.Len == Other.View.Len) && (strncmp(View.Str, Other.View.Str, View.Len) == 0);
		}

		std::size_t operator()(const StringIdHash& Key) const
		{
			return Hash;
		}

		uint64 Hash;
		StringIdView View;
	};

	class StringID
	{
	public:
		StringID(int32 Len, const char* Str)
		{
			const StringIdView View(Str, Len);
			StringIdHash Key = StringIdHash(View);

			auto It = StringIdMap.find(Key);
			if (It != StringIdMap.end())
			{
				Handle = It->second;
			}
			else
			{
				Handle = IdAllocator.Create(View);
				StringIdHash NewHash(Key.Hash, IdAllocator.Resolve(Handle).MakeView());
				StringIdMap[NewHash] = Handle;
			}

			Hash = Key.Hash;
		}

		StringID(const char* Str)
			: StringID(strlen(Str), Str)
		{}

		StringID(const std::string& Str)
			: StringID(Str.size(), Str.c_str())
		{}

		StringID(const StringID& Other)
			: Handle(Other.Handle)
			, Hash(Other.Hash)
		{}

		inline StringIdEntry& GetEntry() const { return IdAllocator.Resolve(Handle); }
		inline bool IsNone() const { return GetEntry().Length == 0; }
		inline uint64 GetHash() const { return Hash; }

		//inline std::size_t operator()() const
		//{
		//	return Hash;
		//}

		inline bool operator==(const StringID& Other) const
		{
			return Handle == Other.Handle;
		}

		inline bool operator!=(const StringID& Other) const
		{
			return !(*this == Other);
		}

		inline bool operator==(const char* Other) const
		{
			if ((Other == nullptr) || (Other[0] == '\0'))
			{
				return IsNone();
			}

			return strncmp(GetEntry().Str, Other, GetEntry().GetLength()) == 0;
		}

		inline bool operator!=(const char* Other) const
		{
			return !(*this == Other);
		}

		//bool IsEqual(const StringID& Other, bool bCaseSensetive = false);
		//int32 Compare(const StringID& Other) const;

		inline std::string ToString() const { return GetEntry().GetStr(); };

		friend class Archive& operator<<(Archive& Ar, const StringID& Value);
		friend class Archive& operator>>(Archive& Ar, StringID& Value);

		static void AutoTest();
		static void LogEntries();

	private:
		StringIdEntryHandle Handle;
		uint64 Hash;

		static StringIdEntryAllocator IdAllocator;
		static std::unordered_map<StringIdHash, StringIdEntryHandle, StringIdHash> StringIdMap;

		static StringID None;
	};

}

template<>
struct std::hash<Drn::StringID>
{
	std::size_t operator()(const Drn::StringID& key) const { return key.GetHash(); }
};