#pragma once

#include "ForwardTypes.h"
#include "Runtime/Engine/Component.h"
#include "Runtime/Core/Delegate.h"

#include <gainput/gainput.h>

namespace Drn
{
	DECLARE_DELEGATE(OnKeyEventDelegate);
	DECLARE_DELEGATE_OneParam(OnAxisEventDelegate, float);

	struct KeyMap
	{
		KeyMap(gainput::DeviceButtonId InDeviceButtonID, uint32 InID)
			: DeviceButtonId(InDeviceButtonID)
			, ID(InID)
		{}

		uint32 ID;
		gainput::DeviceButtonId DeviceButtonId;
	};

	struct AxisMap
	{
		AxisMap(gainput::DeviceButtonId InDeviceButtonID, float InScale, uint32 InID)
			: DeviceButtonId(InDeviceButtonID)
			, Scale(InScale)
			, ID(InID)
		{}

		uint32 ID;
		gainput::DeviceButtonId DeviceButtonId;
		float Scale;
	};

	struct AnalogMap
	{
		AnalogMap(gainput::DeviceButtonId InDeviceButtonID, float InScale, uint32 InID)
			: DeviceButtonId(InDeviceButtonID)
			, Scale(InScale)
			, ID(InID)
		{}

		uint32 ID;
		gainput::DeviceButtonId DeviceButtonId;
		float Scale;
	};

	struct KeyBind
	{
		template<class UserClass, class Func>
		KeyBind(gainput::UserButtonId InID, UserClass* Class, Func&& FPress, Func&& FRelease)
			: UserButtonID(InID)
		{
			PressDel.Bind(Class, FPress);
			ReleaseDel.Bind(Class, FRelease);
		}

		gainput::UserButtonId UserButtonID;
		OnKeyEventDelegate PressDel;
		OnKeyEventDelegate ReleaseDel;
		std::vector<KeyMap> m_KeyMap;
	};

	struct AxisBind
	{
		template<class UserClass, class Func>
		AxisBind(gainput::UserButtonId InID, float InRaiseRate, float InLowerRate, UserClass* Class, Func&& F)
			: UserButtonID(InID)
			, RaiseRate(InRaiseRate)
			, LowerRate(InLowerRate)
			, Value(0.0f)
		{
			Del.Bind(Class, F);
		}

		gainput::UserButtonId UserButtonID;
		float RaiseRate;
		float LowerRate;
		OnAxisEventDelegate Del;
		std::vector<AxisMap> m_AxisMap;
		float Value;
	};

	// TODO: merge this with axis bind
	struct AnalogBind
	{
		template<class UserClass, class Func>
		AnalogBind(gainput::UserButtonId InID, UserClass* Class, Func&& F)
			: UserButtonID(InID)
		{
			Del.Bind(Class, F);
		}

		gainput::UserButtonId UserButtonID;
		OnAxisEventDelegate Del;
		std::vector<AnalogMap> m_AnalogMap;
	};

	class InputComponent : public Component
	{
	public:
		InputComponent();
		virtual ~InputComponent();

		void Tick( float DeltaTime ) override;
		EComponentType GetComponentType() override { return EComponentType::InputComponent; };
		void Serialize( Archive& Ar ) override;
		void DrawDetailPanel( float DeltaTime ) override;
		void RegisterComponent( World* InOwningWorld ) override;
		void UnRegisterComponent() override;
		void DestroyComponent() override;

		bool IsMouseKey( gainput::DeviceButtonId DeviceButtonID ) const;
		
		template<class UserClass, class Func>
		void AddKey(gainput::UserButtonId UserButtonID, UserClass* Class, Func&& FPress, Func&& FRelease);
		void AddKeyMapping(gainput::UserButtonId UserButtonID, gainput::DeviceButtonId DeviceButtonID);

		template<class UserClass, class Func>
		void AddAxis(gainput::UserButtonId UserButtonID, float RaiseRate, float LowerRate, UserClass* Class, Func&& F);
		void AddAxisMapping(gainput::UserButtonId UserButtonID, gainput::DeviceButtonId DeviceButtonID, float Scale);

		template<class UserClass, class Func>
		void AddAnalog(gainput::UserButtonId UserButtonID, UserClass* Class, Func&& F);
		void AddAnalogMapping(gainput::UserButtonId UserButtonID, gainput::DeviceButtonId DeviceButtonID, float Scale);

	protected:

		gainput::InputMap* m_InputMap;

		std::vector<KeyBind> m_KeyBinds;
		std::vector<AxisBind> m_AxisBinds;
		std::vector<AnalogBind> m_AnalogBinds;

		uint32 m_CounterID;
	};

	template<class UserClass, class Func>
	void InputComponent::AddKey( gainput::UserButtonId UserButtonID, UserClass* Class, Func&& FPress, Func&& FRelease)
	{
		auto it = std::find_if(m_KeyBinds.begin(), m_KeyBinds.end(), [&](const KeyBind& Bind){ return Bind.UserButtonID == UserButtonID; });

		if (it == m_KeyBinds.end())
			m_KeyBinds.emplace_back(UserButtonID, Class, FPress, FRelease);
	}

	template<class UserClass, class Func>
	void InputComponent::AddAxis(gainput::UserButtonId UserButtonID, float RaiseRate, float LowerRate, UserClass* Class, Func&& F)
	{
		auto it = std::find_if(m_AxisBinds.begin(), m_AxisBinds.end(), [&](const AxisBind& Bind){ return Bind.UserButtonID == UserButtonID; });

		if (it == m_AxisBinds.end())
			m_AxisBinds.emplace_back(UserButtonID, RaiseRate, LowerRate, Class, F);
	}

	template<class UserClass, class Func>
	void InputComponent::AddAnalog(gainput::UserButtonId UserButtonID, UserClass* Class, Func&& F)
	{
		auto it = std::find_if(m_AnalogBinds.begin(), m_AnalogBinds.end(), [&](const AnalogBind& Bind){ return Bind.UserButtonID == UserButtonID; });

		if (it == m_AnalogBinds.end())
			m_AnalogBinds.emplace_back(UserButtonID, Class, F);
	}
}