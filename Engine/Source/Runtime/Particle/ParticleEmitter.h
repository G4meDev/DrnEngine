#pragma once

#include "ForwardTypes.h"
#include "Runtime/Core/Serializable.h"

namespace Drn
{
	class ParticleEmitter : public Serializable, public RefCountedObject
	{
	public:
		ParticleEmitter();

		virtual void Serialize(Archive& Ar) override;

		inline void SetName(const std::string& InName) { Name = InName; }
		inline std::string& GetName() { return Name; }

		inline bool IsEnabled() const { return bEnabled; }
		inline void SetEnabled(bool bInEnabled) { bEnabled = bInEnabled; }

		std::vector<TRefCountPtr<ParticleModuleSpawnBase>> SpawningModules;
		std::vector<TRefCountPtr<ParticleModule>> SpawnModules;
		std::vector<TRefCountPtr<ParticleModule>> UpdateModules;

	private:
		std::string Name;
		bool bEnabled;
	};
}