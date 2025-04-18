#pragma once

#include "ForwardTypes.h"

#include "SphereElem.h"
#include "BoxElem.h"
#include "CapsuleElem.h"

namespace Drn
{
	class AggregateGeom : public Serializable
	{
	public:

		std::vector<SphereElem>		SphereElems;
		std::vector<BoxElem>		BoxElems;
		std::vector<CapsuleElem>	CapsuleElems;

		// TODO: add shape type
		std::vector<SphereElem> ConvexElems;

		virtual void Serialize(Archive& Ar) override;

		int32 GetElementCount() const 
		{
			return SphereElems.size() + BoxElems.size() + CapsuleElems.size() + ConvexElems.size();
		}

		ShapeElem* GetElement(const EAggCollisionShape Type, const uint32 Index)
		{
			if		(Type == EAggCollisionShape::Sphere)	{ return &SphereElems[Index]; }
			else if (Type == EAggCollisionShape::Box)		{ return &BoxElems[Index]; }
			else if (Type == EAggCollisionShape::Capsule)	{ return &CapsuleElems[Index]; }
			else if (Type == EAggCollisionShape::Convex)	{ return &ConvexElems[Index]; }
			else											{ return nullptr; }
		}

		ShapeElem* GetElement(uint32 InIndex)
		{
			int Index = InIndex;

			if ( Index < SphereElems.size() ) { return &SphereElems[Index]; }
			Index -= SphereElems.size();

			if ( Index < BoxElems.size() ) { return &BoxElems[Index]; }
			Index -= BoxElems.size();

			if ( Index < CapsuleElems.size() ) { return &CapsuleElems[Index]; }
			Index -= CapsuleElems.size();

			if ( Index < ConvexElems.size() ) { return &ConvexElems[Index]; }

			return nullptr;
		}

		void EmptyElements()
		{
			SphereElems.clear();
			BoxElems.clear();
			CapsuleElems.clear();
			ConvexElems.clear();
		}

	private:
	};
}