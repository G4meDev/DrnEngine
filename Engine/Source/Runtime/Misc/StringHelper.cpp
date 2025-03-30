#include "DrnPCH.h"
#include "StringHelper.h"

namespace Drn
{
	std::set<std::string> StringHelper::split( const std::string& str, const std::string& delimiter )
	{
	    std::set<std::string>   tokens;
	    std::regex                 re( delimiter );
	    std::sregex_token_iterator it( str.begin(), str.end(), re, -1 );
	    std::sregex_token_iterator reg_end;
	
	    for ( ; it != reg_end; ++it )
	    {
	        tokens.insert( it->str() );
	    }
	
	    return tokens;
	}

	std::wstring StringHelper::s2ws( const std::string& str )
	{
		using convert_typeX = std::codecvt_utf8<wchar_t>;
		std::wstring_convert<convert_typeX, wchar_t> converterX;

		return converterX.from_bytes( str );
	}

	std::string StringHelper::ws2s( const std::wstring& wstr )
	{
		using convert_typeX = std::codecvt_utf8<wchar_t>;
		std::wstring_convert<convert_typeX, wchar_t> converterX;

		return converterX.to_bytes( wstr );
	}
}