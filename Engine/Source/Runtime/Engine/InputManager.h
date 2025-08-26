#pragma once

#include "ForwardTypes.h"
#include <gainput/gainput.h>

LOG_DECLARE_CATEGORY( LogInputManager);

namespace Drn
{
	enum EEButton
	{
		ButtonToggleListener,
		ButtonToggleMapListener,
		ButtonConfirm,
		ButtonMouseX,
	};

	class InputManager
	{
	public:

		InputManager();
		~InputManager();

		void Init();
		void Shutdown();
		void Tick(float DeltaTime);

		inline static InputManager* Get() { return m_SingletonInstance; }

		inline gainput::InputManager* GetGaInputManager() const { return m_Manager; }
		inline gainput::DeviceId GetMouseID() const { return m_MouseID; }
		inline gainput::DeviceId GetKeyboardID() const { return m_KeyboardID; }

		void HandleMessage(const MSG& msg);

	private:

		gainput::DeviceId m_KeyboardID;
		gainput::DeviceId m_MouseID;

		gainput::InputManager* m_Manager;
		static InputManager* m_SingletonInstance;
	};
}