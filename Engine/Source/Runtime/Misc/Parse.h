#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class Parse
	{
	public:
		static bool Value(const std::string& Stream, const std::string& Match, float& Value);

	};
}