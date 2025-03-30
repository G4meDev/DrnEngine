#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class StringHelper
	{
	public:
		static std::wstring s2ws( const std::string& str );
		static std::string  ws2s( const std::wstring& wstr );
	};
}