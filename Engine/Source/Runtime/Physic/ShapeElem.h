#pragma once

#include "ForwardTypes.h"
#include "PhysicUserData.h"

namespace Drn
{
	enum class EAggCollisionShape : uint8
	{
		Sphere,
		Box,
		Capsule,
		Convex,

		Unkown
	};

	class ShapeElem
	{
	public:

		ShapeElem()
			: Type(EAggCollisionShape::Unkown)
			, UserData(this)
		{
		}

		ShapeElem(EAggCollisionShape InType)
			: Type(InType)
			, UserData(this)
		{
		}

		virtual ~ShapeElem() {};

		const PhysicUserData* GetUserData() const
		{
			PhysicUserData::Set<ShapeElem>((void*)&UserData, const_cast<ShapeElem*>(this));
			return &UserData;
		}

	private:

		EAggCollisionShape Type;
		PhysicUserData UserData;
	};
}