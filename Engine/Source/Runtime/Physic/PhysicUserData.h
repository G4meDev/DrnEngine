#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	enum class EPhysicUserDataType : uint8
	{
		Invalid,
		BodyInstance,
	};

	class PhysicUserData
	{
	protected:
		EPhysicUserDataType Type;
		void* Payload;

	public:
		PhysicUserData()
			: Type(EPhysicUserDataType::Invalid)
			, Payload(nullptr) {}

		PhysicUserData(BodyInstance* InBodyInstance)
			: Type(EPhysicUserDataType::BodyInstance)
			, Payload(InBodyInstance) {}

		template<typename T>
		static T* Get(void* UserData);
	};

	template<>
	inline BodyInstance* PhysicUserData::Get( void* UserData )
	{
		if (UserData && ((PhysicUserData*)UserData)->Type == EPhysicUserDataType::BodyInstance)
		{
			return (BodyInstance*)((PhysicUserData*)UserData)->Payload;
		}
		return nullptr;
	}


}