#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class World
	{
	public:

		World();
		~World();

		void AddStaticMeshCompponent(StaticMeshComponent* InStaticMesh);
		void RemoveStaticMeshCompponent(StaticMeshComponent* InStaticMesh);

		void AddCameraComponent( CameraComponent* InCamera );
		void RemoveCameraComponent( CameraComponent* InCamera );

	protected:

		// @TODO: promote to actor
		std::vector<StaticMeshComponent*> m_StaticMeshComponents;
		std::vector<CameraComponent*>     m_CameraComponents;

		friend Scene;

	private:
	};
}