#pragma once

#if WITH_EDITOR

namespace Drn
{
	enum class EParameterPopupContext
	{
		None	= 0,
		Copy	= 1 << 1,
		Paste	= 1 << 2,

		CopyPaste = Copy | Paste,
	};
}

#endif