#include "DrnPCH.h"
#include "GUID.h"

namespace Drn
{
	Guid Guid::NewGuid()
	{
		Guid Result;
		HRESULT A = CoCreateGuid( (::GUID*)&Result );
		if (A != S_OK)
		{
			__debugbreak();
		}
		
		return Result;
	}

	std::string Guid::ToString() const
	{
		char buffer[36];
		std::snprintf(buffer, sizeof(buffer), "%08X-%08X-%08X-%08X", A, B, C, D);
		return std::string(buffer);
	}
}