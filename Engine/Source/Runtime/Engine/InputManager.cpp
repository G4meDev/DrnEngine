#include "DrnPCH.h"
#include "InputManager.h"

LOG_DEFINE_CATEGORY( LogInputManager, "InputManager" );

namespace Drn
{
	InputManager* InputManager::m_SingletonInstance = nullptr;

	InputManager::InputManager()
		: m_Manager(nullptr)
	{
		m_Manager = new gainput::InputManager(false);
		m_KeyboardID = m_Manager->CreateDevice<gainput::InputDeviceKeyboard>();
		m_MouseID = m_Manager->CreateDevice<gainput::InputDeviceMouse>();

		//m_InputMap->MapFloat(EEButton::ButtonMouseX, m_MouseID, gainput::MouseAxisX);
	}

	InputManager::~InputManager()
	{
		if (m_Manager)
			delete m_Manager;
	}

	void InputManager::Init()
	{
		if (!m_SingletonInstance)
		{
			m_SingletonInstance = new InputManager();
			LOG(LogInputManager, Info, "Initializing input manager.")
		}
	}

	void InputManager::Shutdown()
	{
		if (m_SingletonInstance)
		{
			delete m_SingletonInstance;
			LOG(LogInputManager, Info, "shutting down input manager.")
		}
	}


	void InputManager::Tick( float DeltaTime )
	{
		m_Manager->Update(DeltaTime);
	}

	void InputManager::HandleMessage( const MSG& msg )
	{
		m_Manager->HandleMessage(msg);
	}

}