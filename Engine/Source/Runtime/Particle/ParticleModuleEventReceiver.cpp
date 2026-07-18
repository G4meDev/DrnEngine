#include "DrnPCH.h"
#include "ParticleModuleEventReceiver.h"

#if WITH_EDITOR
#include "Editor/EditorConfig.h"
#include "imgui.h"
#endif

namespace Drn
{
	ParticleModuleEventReceiverBase::ParticleModuleEventReceiverBase()
		: ParticleModule()
		, EventGeneratorType(EParticleEventType::EPET_Any)
		, EventName("None")
	{}

	void ParticleModuleEventReceiverBase::Serialize( Archive& Ar )
	{
		ParticleModule::Serialize(Ar);

		if (Ar.IsLoading())
		{
			Ar >> *(uint32*)&EventGeneratorType;
			Ar >> EventName;
		}
		else
		{
			Ar << (uint32)EventGeneratorType;
			Ar << EventName;
		}
	}

// -----------------------------------------------------------------------------------------------------

	ParticleModuleEventReceiverKillParticles::ParticleModuleEventReceiverKillParticles()
		: ParticleModuleEventReceiverBase()
		, bStopSpawning(false)
	{
		bEventReciverModule = true;
	}

	void ParticleModuleEventReceiverKillParticles::Serialize( Archive& Ar )
	{
		ParticleModuleEventReceiverBase::Serialize(Ar);

		if (Ar.IsLoading())
		{
			Ar >> bStopSpawning;
		}
		else
		{
			Ar << bStopSpawning;
		}
	}

	bool ParticleModuleEventReceiverKillParticles::ProcessParticleEvent( ParticleEmitterInstance* Owner, ParticleEventData& InEvent, float DeltaTime )
	{
		if ((InEvent.EventName == EventName) && ((EventGeneratorType == EPET_Any) || (EventGeneratorType == InEvent.Type)))
		{
			Owner->KillParticlesForced(true);
			if (bStopSpawning == true)
			{
				Owner->bHaltSpawning = true;
			}
			return true;
		}

		return false;
	}

// -----------------------------------------------------------------------------------------------------

	ParticleModuleEventReceiverSpawn::ParticleModuleEventReceiverSpawn()
		: ParticleModuleEventReceiverBase()
		, SpawnCount(new ParticleDistributionFloatConstant(0.0f))
		, bUseParticleTime(false)
		, bUsePSysLocation(false)
		, bInheritVelocity(false)
		, InheritVelocityScale(new ParticleDistributionVectorConstant(Vector::OneVector))
		, bBanPhysicalMaterials(false)
	{
		bEventReciverModule = true;
	}

	void ParticleModuleEventReceiverSpawn::Serialize( Archive& Ar )
	{
		ParticleModuleEventReceiverBase::Serialize(Ar);

		if (Ar.IsLoading())
		{
			InheritVelocityScale = ParticleDistributionVector::Create(Ar);
			SpawnCount = ParticleDistributionFloat::Create(Ar);

			Ar >> bUseParticleTime;
			Ar >> bUsePSysLocation;
			Ar >> bInheritVelocity;
			Ar >> bBanPhysicalMaterials;

			uint8 MaterialsCount;
			Ar >> MaterialsCount;
			PhysicalMaterials.resize(MaterialsCount);
			for (int32 i = 0; i < MaterialsCount; i++)
			{
				std::string Path;
				Ar >> Path;
				PhysicalMaterials[i] = AssetHandle<PhysicalMaterial>(Path);
				PhysicalMaterials[i].Load();
			}
		}
		else
		{
			InheritVelocityScale->Serialize(Ar);
			SpawnCount->Serialize(Ar);

			Ar << bUseParticleTime;
			Ar << bUsePSysLocation;
			Ar << bInheritVelocity;
			Ar << bBanPhysicalMaterials;

			const uint8 MaterialsCount = PhysicalMaterials.size();
			Ar << MaterialsCount;
			for (int32 i = 0; i < MaterialsCount; i++)
			{
				Ar << PhysicalMaterials[i].GetPath();
			}
		}
	}

	bool ParticleModuleEventReceiverSpawn::ProcessParticleEvent( ParticleEmitterInstance* Owner, ParticleEventData& InEvent, float DeltaTime )
	{
		if ((InEvent.EventName == EventName) && ((EventGeneratorType == EPET_Any) || (EventGeneratorType == InEvent.Type)))
		{
			int32 Count = 0;

			switch (InEvent.Type)
			{
			case EPET_Spawn:
			case EPET_Burst:
				Count = Math::RoundToInt(SpawnCount->GetValue(InEvent.EmitterTime, Owner));
				break;
			case EPET_Death:
				{
					ParticleEventDeathData* DeathData = (ParticleEventDeathData*)(&InEvent);
					Count = Math::RoundToInt(SpawnCount->GetValue(bUseParticleTime ? DeathData->ParticleTime : InEvent.EmitterTime, Owner));
				}
				break;
			case EPET_Collision:
				{
					//ParticleEventCollideData* CollideData = (ParticleEventCollideData*)(&InEvent);
					//PhysicalMaterial* PhysMat = CollideData->PhysMat;
					//bool bPhysMatIsAllowed = !PhysMat || (PhysicalMaterials.size() == 0 || PhysicalMaterials.Contains(PhysMat) == !bBanPhysicalMaterials);
					//
					//if (bPhysMatIsAllowed)
					//{
					//	Count = Math::RoundToInt(SpawnCount->GetValue(bUseParticleTime ? CollideData->ParticleTime : InEvent.EmitterTime, Owner));
					//}
				}
				break;
			case EPET_Blueprint:
				{
					ParticleEventKismetData* KismetData = (ParticleEventKismetData*)(&InEvent);
					Count = Math::RoundToInt(SpawnCount->GetValue(InEvent.EmitterTime, Owner));
				}
				break;
			}

			if (Count > 0)
			{
				Vector SpawnLocation = bUsePSysLocation ? Owner->Location : InEvent.Location;
				Vector Velocity = bInheritVelocity ? 
					(InEvent.Velocity * InheritVelocityScale->GetValue(InEvent.EmitterTime, Owner)) : Vector::ZeroVector;
			
				Owner->ForceSpawn(DeltaTime, 0, Count, SpawnLocation, Velocity);
			}

			return true;
		}

		return false;
	}


// -----------------------------------------------------------------------------------------------------

#if WITH_EDITOR
	bool ParticleModuleEventReceiverBase::Draw( ParticleEmitter* Owner )
	{
		bool bDirty = ParticleModule::Draw(Owner);

		const char* const Options[] = { "Any", "Spawn", "Death", "Collision", "Burst", "Blueprint" };
		int32 Selected = EventGeneratorType;
		bDirty |= ImGui::Combo("Generator Type", &Selected, Options, _countof(Options));
		if (bDirty)
		{
			EventGeneratorType = (EParticleEventType)Selected;
		}

		const int32 TextCharLimit = 64;
		char InputText[TextCharLimit];
		strcpy_s(InputText, sizeof(InputText), EventName.c_str());

		if ( ImGui::InputText( "Event Name", InputText, TextCharLimit ) )
		{
			EventName = InputText;
			bDirty = true;
		}

		return bDirty;
	}

	bool ParticleModuleEventReceiverKillParticles::Draw( ParticleEmitter* Owner )
	{
		bool bDirty = ParticleModuleEventReceiverBase::Draw(Owner);

		bDirty |= ImGui::Checkbox("Stop Spawning", &bStopSpawning);
		
		return bDirty;
	}

	bool ParticleModuleEventReceiverSpawn::Draw( ParticleEmitter* Owner )
	{
		bool bDirty = ParticleModuleEventReceiverBase::Draw(Owner);

		bDirty |= SpawnCount->Draw(SpawnCount, "Spawn Count");
		bDirty |= ImGui::Checkbox("Use Particle Time", &bUseParticleTime);
		bDirty |= ImGui::Checkbox("Use PSys Location", &bUsePSysLocation);
		bDirty |= ImGui::Checkbox("Inherit Velocity", &bInheritVelocity);
		bDirty |= InheritVelocityScale->Draw(InheritVelocityScale, "Inherit Velocity Scale");
		bDirty |= ImGui::Checkbox("Ban Physical Materials", &bBanPhysicalMaterials);

		if (ImGui::Button("Add Material"))
		{
			PhysicalMaterials.push_back({});
			bDirty = true;
		}

		if (ImGui::Button("Clear Material"))
		{
			PhysicalMaterials.clear();
			bDirty = true;
		}

		for (int32 MaterialIndex = 0; MaterialIndex < PhysicalMaterials.size(); MaterialIndex++)
		{
			ImGui::PushID(MaterialIndex);

			std::string AssetPath	= PhysicalMaterials[MaterialIndex].GetPath();
			std::string AssetName	= Path::ConvertShortPath(AssetPath);
			AssetName				= Path::RemoveFileExtension(AssetName);
			AssetName				= AssetName == "" ? "None" : AssetName;

			ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Text, EditorConfig::AssetInputColor);
			ImGui::Text( "%s", AssetName.c_str() );
			ImGui::PopStyleColor();

			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(EditorConfig::Payload_AssetPath()))
				{
					auto AssetPath = static_cast<const char*>(payload->Data);
				
					AssetHandle<Asset> NewMaterial(AssetPath);
					EAssetType Type = NewMaterial.LoadGeneric();

					if (NewMaterial.IsValid() && Type == EAssetType::PhysicalMaterial)
					{
						AssetHandle<PhysicalMaterial> MatAsset(AssetPath);
						MatAsset.Load();

						PhysicalMaterials[MaterialIndex] = MatAsset;
					}
				}

				ImGui::EndDragDropTarget();
			}

			ImGui::Separator();
			ImGui::TextWrapped(PhysicalMaterials[MaterialIndex].GetPath().c_str());
			ImGui::Separator();

			ImGui::PopID();
		}
		
		return bDirty;
	}
#endif

}  // namespace Drn