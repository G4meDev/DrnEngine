#pragma once

#include "ForwardTypes.h"
#include "AggregateGeom.h"

namespace Drn
{
	class BodySetup : public Serializable
	{
	public:

		BodySetup() {}
		virtual ~BodySetup() {}

		virtual void Serialize( Archive& Ar ) override;

		AggregateGeom m_AggGeo;

		// TODO: Add physic material

#if WITH_EDITOR
		void DrawDetailPanel();

		void DrawSphereShapes();
		void DrawBoxShapes();
		void DrawCapsuleShapes();
#endif

	private:

		
	};
}