#pragma once

#if WITH_EDITOR

#include "Runtime/Core/Delegate.h"

namespace Drn
{
	DECLARE_DELEGATE(OnLayerCloseDelegate);

	class ImGuiLayer
	{
	public:
		ImGuiLayer();
		virtual ~ImGuiLayer();

		virtual void Draw(float DeltaTime) = 0;

		void Attach();
		void DeAttach();

		bool m_Open;

		OnLayerCloseDelegate OnLayerClose;
	};
}

#endif