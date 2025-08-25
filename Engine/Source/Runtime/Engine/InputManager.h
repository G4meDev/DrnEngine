#pragma once

#include "ForwardTypes.h"
#include <gainput/gainput.h>

LOG_DECLARE_CATEGORY( LogInputManager);

namespace Drn
{
	class InputManager
	{
	public:

		InputManager();
		~InputManager();

		void Init();
		void Shutdown();

		inline static InputManager* Get() { return m_SingletonInstance; }

		void HandleMessage(const MSG& msg);

	private:

		gainput::InputManager m_Manager;
		static InputManager* m_SingletonInstance;

	};
}