#pragma once

#include "ForwardTypes.h"
#include "AggregateGeom.h"

namespace Drn
{
	class BodySetup
	{
	public:

		BodySetup() {}
		virtual ~BodySetup() {}

		AggregateGeom AggGeo;

		// TODO: Add physic material

	private:

		
	};
}