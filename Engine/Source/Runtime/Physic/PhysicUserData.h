#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	enum class EPhysicUserDataType : uint8
	{
		Invalid,
		BodyInstance,
		AggShape
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

		PhysicUserData(class ShapeElem* InShape)
			: Type(EPhysicUserDataType::AggShape)
			, Payload(InShape) {}

		template<typename T>
		static T* Get(void* UserData);

		template<class T>
		static void Set( void* UserData, T* Payload );
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

	//template<>
	//inline BodyInstance* PhysicUserData::Get( void* UserData )
	//{
	//	if (UserData && ((PhysicUserData*)UserData)->Type == EPhysicUserDataType::BodyInstance)
	//	{
	//		return (BodyInstance*)((PhysicUserData*)UserData)->Payload;
	//	}
	//	return nullptr;
	//}

	template<>
	inline void PhysicUserData::Set( void* UserData, class ShapeElem* Payload )
	{
		((PhysicUserData*)UserData)->Type = EPhysicUserDataType::AggShape;
		((PhysicUserData*)UserData)->Payload = Payload;
	}
}