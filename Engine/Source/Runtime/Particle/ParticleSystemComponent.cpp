#include "DrnPCH.h"
#include "ParticleSystemComponent.h"

#if WITH_EDITOR
#include "Editor/EditorConfig.h"
#endif

namespace Drn
{
	ParticleSystemComponent::ParticleSystemComponent()
		: bDeactivateTriggered(false)
		, bWasCompleted(false)
		, bWasDeactivated(false)
		, bSuppressSpawning(false)
		, bWasActive(false)
		, TotalActiveParticles(0)
		, NumSignificantEmitters(0)
	{
		bTickInEditor = true;
		
	}

	ParticleSystemComponent::~ParticleSystemComponent()
	{
		
	}

	void ParticleSystemComponent::Tick( float DeltaTime )
	{
		SceneComponent::Tick(DeltaTime);

		if (bDeactivateTriggered)
		{
			DeactivateSystem();
		}

		if (bWasCompleted)
		{
			return;
		}

		NumSignificantEmitters = 0;
		TotalActiveParticles = 0;

		for (int32 EmitterIndex = 0; EmitterIndex < Emitters.size(); EmitterIndex++)
		{
			ParticleEmitterInstance* Instance = Emitters[EmitterIndex];

			if (EmitterIndex + 1 < Emitters.size())
			{
				ParticleEmitterInstance* NextInstance = Emitters[EmitterIndex+1];
				ApplicationMisc::Prefetch(NextInstance);
			}

			if (Instance && Instance->Emitter)
			{
				if (Instance->bEnabled)
				{
					Instance->Tick(DeltaTime, bSuppressSpawning);

					NumSignificantEmitters++;
					TotalActiveParticles += Instance->ActiveParticles;
				}
			}
		}

		//if (FXConsoleVariables::bFreezeParticleSimulation == false)
		//{
		//	int32 EmitterIndex;
		//	// Now, process any events that have occurred.
		//	for (EmitterIndex = 0; EmitterIndex < EmitterInstances.Num(); EmitterIndex++)
		//	{
		//		FParticleEmitterInstance* Instance = EmitterInstances[EmitterIndex];
		//		if (Instance && Instance->bEnabled)
		//		{
		//			if (EmitterIndex + 1 < EmitterInstances.Num())
		//			{
		//				FParticleEmitterInstance* NextInstance = EmitterInstances[EmitterIndex+1];
		//				FPlatformMisc::Prefetch(NextInstance);
		//			}
		//
		//			if (Instance->SpriteTemplate)
		//			{
		//				UParticleLODLevel* SpriteLODLevel = Instance->SpriteTemplate->GetCurrentLODLevel(Instance);
		//				if (SpriteLODLevel && SpriteLODLevel->bEnabled)
		//				{
		//					Instance->ProcessParticleEvents(DeltaTimeTick, bSuppressSpawning);
		//				}
		//			}
		//		}
		//	}
		//
		//	UWorld* World = GetWorld();
		//	AParticleEventManager* EventManager = (World ? World->MyParticleEventManager : NULL);
		//	if (EventManager)
		//	{
		//		if (SpawnEvents.Num() > 0) EventManager->HandleParticleSpawnEvents(this, SpawnEvents);
		//		if (DeathEvents.Num() > 0) EventManager->HandleParticleDeathEvents(this, DeathEvents);
		//		if (CollisionEvents.Num() > 0) EventManager->HandleParticleCollisionEvents(this, CollisionEvents);
		//		if (BurstEvents.Num() > 0) EventManager->HandleParticleBurstEvents(this, BurstEvents);
		//	}
		//}

		const bool bIsCompleted = HasCompleted();
		if (bIsCompleted && !bWasCompleted)
		{
			Complete();
		}
		bWasCompleted = bIsCompleted;

		// Update bounding box.
		//if (!bWarmingUp && !bWasCompleted && !Template->bUseFixedRelativeBoundingBox && !bIsTransformDirty)
		//{
		//	// Force an update every once in a while to shrink the bounds.
		//	TimeSinceLastForceUpdateTransform += DeltaTimeTick;
		//	if(TimeSinceLastForceUpdateTransform > MaxTimeBeforeForceUpdateTransform)
		//	{
		//		bIsTransformDirty = true;
		//	}
		//	else
		//	{
		//		// Compute the new system bounding box.
		//		FBox BoundingBox;
		//		BoundingBox.Init();
		//
		//		for (int32 i=0; i<EmitterInstances.Num(); i++)
		//		{
		//			FParticleEmitterInstance* Instance = EmitterInstances[i];
		//			if (Instance && Instance->SpriteTemplate)
		//			{
		//				UParticleLODLevel* SpriteLODLevel = Instance->SpriteTemplate->GetCurrentLODLevel(Instance);
		//				if (SpriteLODLevel && SpriteLODLevel->bEnabled)
		//				{
		//					BoundingBox += Instance->GetBoundingBox();
		//				}
		//			}
		//		}
		//
		//		// Only update the primitive's bounding box in the octree if the system bounding box has gotten larger.
		//		if(!Bounds.GetBox().IsInside(BoundingBox.Min) || !Bounds.GetBox().IsInside(BoundingBox.Max))
		//		{
		//			bIsTransformDirty = true;
		//		}
		//	}
		//}
		//
		//// Update if the component transform has been dirtied.
		//if(bIsTransformDirty)
		//{
		//	UpdateComponentToWorld();
		//
		//	TimeSinceLastForceUpdateTransform = 0.0f;
		//	bIsTransformDirty = false;
		//}
		//
		//if (bOldPositionValid)
		//{
		//	const float InvDeltaTime = (DeltaTimeTick > 0.0f) ? 1.0f / DeltaTimeTick : 0.0f;
		//	PartSysVelocity = (GetComponentLocation() - OldPosition) * InvDeltaTime;
		//}
		//else
		//{
		//	PartSysVelocity = FVector::ZeroVector;
		//}
		//bOldPositionValid = true;
		//OldPosition = GetComponentLocation();
		//
		//if (bIsViewRelevanceDirty)
		//{
		//	ConditionalCacheViewRelevanceFlags();
		//}
		//
		//if (bSkipUpdateDynamicDataDuringTick == false)
		//{
		//	Super::MarkRenderDynamicDataDirty();
		//}
	}

	void ParticleSystemComponent::Serialize( Archive& Ar )
	{
		SceneComponent::Serialize(Ar);

		if ( Ar.IsLoading() )
		{
			std::string TemplatePath = "";
			Ar >> TemplatePath;
			SetTemplate(AssetHandle<ParticleSystem>(TemplatePath));
		}

		else
		{
			Ar << Template.GetPath();
		}

	}

	void ParticleSystemComponent::RegisterComponent( World* InOwningWorld )
	{
		SceneComponent::RegisterComponent(InOwningWorld);

#if WITH_EDITOR
		AssetHandle<Texture2D> DefaultIcon( "Engine\\Content\\EditorResources\\ComponentIcons\\T_ParticleIcon.drn" );
		DefaultIcon.Load();
		
		m_Sprite->SetSprite( DefaultIcon );
#endif
	}

	void ParticleSystemComponent::UnRegisterComponent()
	{
		SceneComponent::UnRegisterComponent();
	}

	void ParticleSystemComponent::SetTemplate( AssetHandle<ParticleSystem> InTemplate )
	{
		Template = InTemplate;
		Template.Load();

		bWasCompleted = false;
		bWasActive = IsActive() && !bWasDeactivated;

		ResetParticles(true);

		if (Template.IsValid())
		{
			if (ShouldAutoActivate() || bWasActive)
			{
				ActivateSystem();
			}
			else
			{
				InitParticles();
			}
		}
	}

	bool ParticleSystemComponent::IsUsingTemplate( AssetHandle<ParticleSystem> InTemplate )
	{
		return Template.IsValid() && (Template.GetPath() == InTemplate.GetPath());
	}

	void ParticleSystemComponent::InitParticles()
	{
		//Emitters.clear();

		if (Template.IsValid())
		{
			int32 NumInstances = Emitters.size();
			int32 NumEmitters = Template->Emitters.size();
			const bool bIsFirstCreate = NumInstances == 0;
			Emitters.resize(NumEmitters);

			bWasCompleted = bIsFirstCreate ? false : bWasCompleted;

			for (int32 i = 0; i < NumEmitters; i++)
			{
				ParticleEmitter* Emitter = Template->Emitters[i];
				if (Emitter && Emitter->IsEnabled())
				{
					ParticleEmitterInstance* Instance = NumInstances == 0 ? NULL : Emitters[i];

					if (Instance)
					{
						//Instance->SetHaltSpawning(false);
					}
					else
					{
						TRefCountPtr<ParticleMeshEmitterInstance> MeshEmitter = new ParticleMeshEmitterInstance();
						Instance = Emitters[i] = ((ParticleEmitterInstance*)MeshEmitter);
					}

					if (Instance)
					{
						Instance->bEnabled = true;
						Instance->InitParameters(Emitter, this);
						Instance->Init();
					}
				}
			}
		}
	}

	bool ParticleSystemComponent::HasCompleted()
	{
		bool bHasCompleted = true;
		bool bCanBeDeactivated = true;

		bool bClearDynamicData = false;
		for (int32 i=0; i<Emitters.size(); i++)
		{
			ParticleEmitterInstance* Instance = Emitters[i];

			if (Instance && Instance->bEnabled)
			{
				if (!Instance->bEmitterIsDone)
				{
					bCanBeDeactivated = false;
				}

				if (Instance->Emitter->EmitterLoops > 0)
				{
					if (bWasDeactivated && bSuppressSpawning)
					{
						if (Instance->ActiveParticles != 0)
						{
							bHasCompleted = false;
						}
					}
					else
					{
						if (Instance->HasCompleted())
						{
							if (Instance->Emitter->bKillOnCompleted)
							{
								// clean up other instances that may point to this one
								for (int32 InnerIndex=0; InnerIndex < Emitters.size(); InnerIndex++)
								{
									if (InnerIndex != i && Emitters[InnerIndex] != NULL)
									{
										//Emitters[InnerIndex]->OnEmitterInstanceKilled(Instance);
									}
								}
								Emitters[i] = nullptr;
								bClearDynamicData = true;
							}
						}
						else
						{
							bHasCompleted = false;
						}
					}
				}
				else
				{
					if (bWasDeactivated)
					{
						if (Instance->ActiveParticles != 0)
						{
							bHasCompleted = false;
						}
					}
					else
					{
						bHasCompleted = false;
					}
				}

			}
		}

		if (bCanBeDeactivated && Template.IsValid() && Template->bAutoDeactivate)
		{
			DeactivateSystem();
		}

		if (bClearDynamicData)
		{
			//ClearDynamicData();
		}
	
		return bHasCompleted;
	}

	void ParticleSystemComponent::Complete()
	{
		ResetParticles();
	}

	void ParticleSystemComponent::ResetParticles(bool bEmptyInstances)
	{
		if (bEmptyInstances)
		{
			Emitters.clear();
			//ClearDynamicData();
		}
		else
		{
			for (int32 EmitterIndex = 0; EmitterIndex < Emitters.size(); EmitterIndex++)
			{
				ParticleEmitterInstance* EmitInst = Emitters[EmitterIndex];
				if (EmitInst)
				{
					EmitInst->Rewind();
				}
			}
		}

		//MarkRenderStateDirty();
		SetActive(false);
	}

	bool ParticleSystemComponent::ShouldActivate()
	{
		return !IsActive() || bWasDeactivated || bWasCompleted;
	}

	void ParticleSystemComponent::Deactivate()
	{
		if (!ShouldActivate())
		{
			DeactivateSystem();

			if (bWasDeactivated)
			{
				//OnComponentDeactivated.Broadcast(this);
			}
		}
	}

	void ParticleSystemComponent::DeactivateSystem()
	{
		bDeactivateTriggered = false;
		bSuppressSpawning = true;
		bWasDeactivated = true;

		bool bShouldMarkRenderStateDirty = false;
		for (int32 i = 0; i < Emitters.size(); i++)
		{
			ParticleEmitterInstance* Instance = Emitters[i];
			if (Instance)
			{
				if (Instance->Emitter->bKillOnDeactivate)
				{
					// clean up other instances that may point to this one
					for (int32 InnerIndex=0; InnerIndex < Emitters.size(); InnerIndex++)
					{
						if (InnerIndex != i && Emitters[InnerIndex] != NULL)
						{
							//Emitters[InnerIndex]->OnEmitterInstanceKilled(Instance);
						}
					}
					Emitters[i] = nullptr;
					bShouldMarkRenderStateDirty = true;
				}
				else
				{
					//Instance->OnDeactivateSystem();
				}
			}
		}

		if (bShouldMarkRenderStateDirty)
		{
			//ClearDynamicData();
			//MarkRenderStateDirty();
		}
	}

	void ParticleSystemComponent::Activate()
	{
		if (Template.IsValid())
		{
			bDeactivateTriggered = false;

			if (ShouldActivate())
			{
				ActivateSystem();

				if (IsActive())
				{
					//OnComponentActivated.Broadcast(this, bReset);
				}
			}
		}
	}

	void ParticleSystemComponent::ActivateSystem()
	{
		//bOldPositionValid = false;
		//OldPosition = FVector::ZeroVector;
		//PartSysVelocity = FVector::ZeroVector;

		if( Template.IsValid() )
		{
			bSuppressSpawning = false;

			bool bNeedToUpdateTransform = bWasDeactivated;
			bWasCompleted = false;
			bWasDeactivated = false;
			SetActive(true);
			bWasActive = false;
			//SetComponentTickEnabled(true);

			InitParticles();
		}

		//MarkRenderStateDirty();
	}

	BoxSphereBounds ParticleSystemComponent::CalcBounds( const Transform& LocalToWorld ) const
	{
		Box BoundingBox;
		BoundingBox.Init();

		const bool bFixedBounds = Template.IsValid() && Template->bUseFixedBounds;
		if(bFixedBounds)
		{
			BoundingBox	= Box(Template->FixedBoundsMin, Template->FixedBoundsMax);
			return BoxSphereBounds(BoundingBox).TransformBy(LocalToWorld);
		}
		else
		{
			for (int32 i=0; i<Emitters.size(); i++)
			{
				ParticleEmitterInstance* EmitterInstance = Emitters[i];
				if( EmitterInstance && EmitterInstance->HasActiveParticles() )
				{
					BoundingBox += EmitterInstance->GetBoundingBox();
				}
			}

			if (!BoundingBox.bValid)
			{
				return BoxSphereBounds(LocalToWorld.GetLocation(), Vector(0.001f), 0.0f);
			}

			const Vector ExpandAmount = BoundingBox.GetExtent() * 0.1f;
			BoundingBox = Box(BoundingBox.Min - ExpandAmount, BoundingBox.Max + ExpandAmount);

			return BoxSphereBounds(BoundingBox);
		}
	}

#if WITH_EDITOR
	void ParticleSystemComponent::DrawDetailPanel( float DeltaTime )
	{
		if ( ImGui::Button( "Clear" ) )
		{
			SetTemplate(AssetHandle<ParticleSystem>(""));
		}

		std::string AssetPath	= Template.GetPath();
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
				AssetHandle<Asset> NewAsset(AssetPath);
				EAssetType Type = NewAsset.LoadGeneric();

				if (NewAsset.IsValid() && Type == EAssetType::ParticleSystem)
				{
					AssetHandle<ParticleSystem> TypedAsset(AssetPath);
					TypedAsset.Load();

					SetTemplate(TypedAsset);
				}
			}

			ImGui::EndDragDropTarget();
		}

		ImGui::Separator();
		ImGui::TextWrapped(Template.GetPath().c_str());

		if (ImGui::Button("Deactivate"))
		{
			Deactivate();
		} ImGui::SameLine();

		if (ImGui::Button("Activate"))
		{
			Activate();
		} ImGui::SameLine();
	}

	void ParticleSystemComponent::DrawEditorDefault()
	{
		
	}

	void ParticleSystemComponent::DrawEditorSelected()
	{
		const bool bUseFixedBound = Template.IsValid() && Template->bUseFixedBounds;
		if (bUseFixedBound)
		{
			Box LocalFixedBound = Box(Template->FixedBoundsMin, Template->FixedBoundsMax);
			GetWorld()->DrawDebugBox(LocalFixedBound, GetWorldTransform(), Color::Blue, 0.0f, 0.0f);
		}

		BoxSphereBounds Bounds = CalcBounds(GetWorldTransform());
		GetWorld()->DrawDebugBox(Box(Bounds.BoxExtent * -1, Bounds.BoxExtent), Transform(Bounds.Origin, Quat::Identity), Color::White, 0.0f, 0.0f);
	}
#endif
}  // namespace Drn