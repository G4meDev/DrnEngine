#include "DrnPCH.h"
#include "StringID.h"

namespace Drn
{
	StringIdEntryAllocator StringID::IdAllocator;
	std::unordered_map<Drn::StringIdHash, Drn::StringIdEntryHandle, Drn::StringIdHash> StringID::StringIdMap(2048);

	StringID StringID::None(0, "");

	Archive& operator<<(Archive& Ar, const StringID& Value)
	{
		Ar << Value.ToString();
		return Ar;
	}

	Archive& operator>>(Archive& Ar, StringID& Value)
	{
		std::string Str;
		Ar >> Str;
		Value = StringID(Str);

		return Ar;
	}

	void StringID::AutoTest()
	{
		StringID None("");
		StringID NoneCopy(StringID::None);

		StringID Test1("Test_1");
		StringID Test2("Test_2");
		StringID Test3("Test_3");

		StringID Test11("Test_11");

		drn_check(Test1 == Test1);
		drn_check(Test1 != Test11);
		drn_check(Test11 != Test1);

		drn_check(None.IsNone());
		drn_check(None == StringID::None);
		drn_check(NoneCopy == StringID::None);

		drn_check(StringID("Test") != StringID("TEST"));
		drn_check(StringID("Test") == StringID(std::string("Test")));
		drn_check(StringID("Test") == StringID(4, "Test"));

		const int32 TestCount = 1000;
		for (int32 Index = 0; Index < TestCount; Index++)
		{
			std::string GeneratedStr = std::format("Generated StringId {}", Index);
			{
				StringID TestStr(GeneratedStr);
			}

			drn_check(StringID(GeneratedStr).ToString() == GeneratedStr);
			drn_check(StringID(GeneratedStr) == GeneratedStr.c_str());
		}

	}

	void StringID::ListNames()
	{
		for (const auto& It : StringIdMap)
		{
			StringID Str(It.first.View.Len, It.first.View.Str);

			std::cout << std::format("Hash: {:<22} | Block: ({:<3}, {:<4}) | String: {}", It.first.Hash, It.second.Block, It.second.Offset, Str.ToString()) << "\n";
		}

		std::cout << "StringID Map Load: " << StringIdMap.load_factor() << "\n";
	}


}  // namespace Drn