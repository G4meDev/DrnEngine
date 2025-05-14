#pragma once

#include "ForwardTypes.h"
#include "PhysicUserData.h"
#include "PhysicCore.h"

using namespace physx;

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

	class ShapeElem : public Serializable
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

		virtual void Serialize(Archive& Ar) override;

		template <typename T>
		T* GetShape()
		{
			return (T*)this;
		}

		PhysicUserData& GetUserData()
		{
			PhysicUserData::Set<ShapeElem>((void*)&UserData, const_cast<ShapeElem*>(this));
			return UserData;
		}

		inline virtual std::shared_ptr<PxGeometry> GetPxGeometery(const Vector& Scale) = 0;

		inline EAggCollisionShape GetType() const { return Type; }

	private:

		EAggCollisionShape Type;
		PhysicUserData UserData;
	};
}