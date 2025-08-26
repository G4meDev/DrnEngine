#include "DrnPCH.h"
#include "InputComponent.h"

#include <imgui.h>

namespace Drn
{
	InputComponent::InputComponent()
		: Component()
		, m_InputMap(nullptr)
		, m_CounterID(0)
	{
	}

	InputComponent::~InputComponent()
	{
	}

	void InputComponent::Tick( float DeltaTime )
	{
		Component::Tick(DeltaTime);

		for (int32 i = 0; i < m_KeyBinds.size(); i++)
		{
			auto& KeyMaps = m_KeyBinds[i].m_KeyMap;

			bool HasPressEvent = false;
			bool HasReleaseEvent = false;

			for (int32 j = 0; j < KeyMaps.size(); j++)
			{
				HasPressEvent |= m_InputMap->GetBoolIsNew(KeyMaps[j].ID);
				HasReleaseEvent |= m_InputMap->GetBoolWasDown(KeyMaps[j].ID);
			}

			if (HasReleaseEvent)
				m_KeyBinds[i].ReleaseDel.Execute();

			if (HasPressEvent)
				m_KeyBinds[i].PressDel.Execute();
		}

		for (int32 i = 0; i < m_AxisBinds.size(); i++)
		{
			auto& AxisMaps = m_AxisBinds[i].m_AxisMap;
			float NewValue = 0;
			for (int32 j = 0; j < AxisMaps.size(); j++)
			{
				NewValue += m_InputMap->GetBool(AxisMaps[j].ID) * AxisMaps[j].Scale;
			}

			// TODO: raise and lower

			if (NewValue != 0)
			{
				m_AxisBinds[i].Del.Execute(NewValue);
			}
		}

		for (int32 i = 0; i < m_AnalogBinds.size(); i++)
		{
			auto& AxisMaps = m_AnalogBinds[i].m_AnalogMap;
			float NewValue = 0;
			for (int32 j = 0; j < AxisMaps.size(); j++)
			{
				NewValue += m_InputMap->GetFloatDelta(AxisMaps[j].ID) * AxisMaps[j].Scale;
			}

			if (NewValue != 0)
			{
				m_AnalogBinds[i].Del.Execute(NewValue);
			}
		}
	}

	void InputComponent::Serialize( Archive& Ar )
	{
		if (Ar.IsLoading())
		{
			
		}

		else
		{
			
		}
	}

	void InputComponent::RegisterComponent( World* InOwningWorld )
	{
		Component::RegisterComponent(InOwningWorld);

		m_InputMap = new gainput::InputMap( *InputManager::Get()->GetGaInputManager(), "Default" );
	}
	
	void InputComponent::UnRegisterComponent()
	{
		if (m_InputMap)
		{
			m_InputMap->Clear();
			delete m_InputMap;
		}

		Component::UnRegisterComponent();
	}
	
	void InputComponent::DestroyComponent()
	{
		Component::DestroyComponent();


	}

	bool InputComponent::IsMouseKey( gainput::DeviceButtonId DeviceButtonID ) const
	{
		return (DeviceButtonID == gainput::MouseButtonLeft
			|| DeviceButtonID == gainput::MouseButtonRight
			|| DeviceButtonID == gainput::MouseAxisX
			|| DeviceButtonID == gainput::MouseAxisY
		);
	}

	void InputComponent::AddKeyMapping( gainput::UserButtonId UserButtonID, gainput::DeviceButtonId DeviceButtonID )
	{
		auto it = std::find_if(m_KeyBinds.begin(), m_KeyBinds.end(), [&](const KeyBind& Bind){ return Bind.UserButtonID == UserButtonID; });

		if (it != m_KeyBinds.end())
		{
			auto it2 = std::find_if(it->m_KeyMap.begin(), it->m_KeyMap.end(), [&](const KeyMap& Map){ return Map.DeviceButtonId == DeviceButtonID; });
			if (it2 == it->m_KeyMap.end())
			{
				uint32 ID = m_CounterID++;
				it->m_KeyMap.emplace_back(DeviceButtonID, ID);
				m_InputMap->MapBool(ID, IsMouseKey(DeviceButtonID) ? InputManager::Get()->GetMouseID() : InputManager::Get()->GetKeyboardID(), DeviceButtonID);
			}
		}
	}

	void InputComponent::AddAxisMapping( gainput::UserButtonId UserButtonID, gainput::DeviceButtonId DeviceButtonID, float Scale )
	{
		auto it = std::find_if(m_AxisBinds.begin(), m_AxisBinds.end(), [&](const AxisBind& Bind){ return Bind.UserButtonID == UserButtonID; });

		if (it != m_AxisBinds.end())
		{
			auto it2 = std::find_if(it->m_AxisMap.begin(), it->m_AxisMap.end(), [&](const AxisMap& Map){ return Map.DeviceButtonId == DeviceButtonID; });
			if (it2 == it->m_AxisMap.end())
			{
				uint32 ID = m_CounterID++;
				it->m_AxisMap.emplace_back(DeviceButtonID, Scale, ID);
				m_InputMap->MapBool(ID, IsMouseKey(DeviceButtonID) ? InputManager::Get()->GetMouseID() : InputManager::Get()->GetKeyboardID(), DeviceButtonID);
			}
		}
	}

	void InputComponent::AddAnalogMapping( gainput::UserButtonId UserButtonID, gainput::DeviceButtonId DeviceButtonID, float Scale )
	{
		auto it = std::find_if(m_AnalogBinds.begin(), m_AnalogBinds.end(), [&](const AnalogBind& Bind){ return Bind.UserButtonID == UserButtonID; });

		if (it != m_AnalogBinds.end())
		{
			auto it2 = std::find_if(it->m_AnalogMap.begin(), it->m_AnalogMap.end(), [&](const AnalogMap& Map){ return Map.DeviceButtonId == DeviceButtonID; });
			if (it2 == it->m_AnalogMap.end())
			{
				uint32 ID = m_CounterID++;
				it->m_AnalogMap.emplace_back(DeviceButtonID, Scale, ID);
				m_InputMap->MapFloat(ID, IsMouseKey(DeviceButtonID) ? InputManager::Get()->GetMouseID() : InputManager::Get()->GetKeyboardID(), DeviceButtonID);
			}
		}
	}

#if WITH_EDITOR
	void InputComponent::DrawDetailPanel( float DeltaTime )
	{
		Component::DrawDetailPanel(DeltaTime);

	}
#endif

}