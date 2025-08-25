#include "DrnPCH.h"
#include "InputManager.h"

LOG_DEFINE_CATEGORY( LogInputManager, "InputManager" );

namespace Drn
{
	InputManager* InputManager::m_SingletonInstance = nullptr;

	InputManager::InputManager()
	{}

	InputManager::~InputManager()
	{}

	void InputManager::Init()
	{
		if (!m_SingletonInstance)
		{
			m_SingletonInstance = new InputManager();
			LOG(LogPhysicManager, Info, "Initializing input manager.")
		}
	}

	void InputManager::Shutdown()
	{
		if (m_SingletonInstance)
		{
			delete m_SingletonInstance;
			LOG(LogPhysicManager, Info, "shutting down input manager.")
		}
	}


	void InputManager::HandleMessage( const MSG& msg )
	{
		m_Manager.HandleMessage(msg);
	}

}