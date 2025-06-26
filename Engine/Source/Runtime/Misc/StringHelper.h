#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class StringHelper
	{
	public:
		static std::set<std::string> split( const std::string& str, const std::string& delimiter );

		static void RemoveWhitespaces( std::string& Str );

		static std::wstring s2ws( const std::string& str );
		static std::string  ws2s( const std::wstring& wstr );
	};
}