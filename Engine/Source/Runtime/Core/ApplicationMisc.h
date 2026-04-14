#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class ApplicationMisc
	{
	public:
		static void ClipboardCopy(const std::wstring& Str);
		static void ClipboardPaste(std::wstring& Result);
	};
}