#include "DrnPCH.h"
#include "Parse.h"

namespace Drn
{
	bool Parse::Value( const std::string& Stream, const std::string& Match, float& Value )
	{
		auto Index = Stream.find(Match);
		if (Index == std::string::npos)
		{
			return false;
		}

		Value = std::atof(Match.size() + Stream.c_str() + Index);
		return true;
	}
}