#pragma once

#include "ForwardTypes.h"

#define PLATFORM_CACHE_LINE_SIZE 64

namespace Drn
{
	class ApplicationMisc
	{
	public:
		static void ClipboardCopy(const std::wstring& Str);
		static void ClipboardPaste(std::wstring& Result);

		inline static void Prefetch(void const* x, int32 offset = 0)
		{
			 _mm_prefetch( (char const*)(x) + offset, _MM_HINT_T0 );
		}

		static uint32 GetUniqueId() { return LastGrabedId++; }

	private:
		static uint32 LastGrabedId; // only for runtime
	};
}