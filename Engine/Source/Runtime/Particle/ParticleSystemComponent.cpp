#include "DrnPCH.h"
#include "ParticleSystemComponent.h"
#include "Runtime/Particle/ParticleEmitterType.h"

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
		, bWarmingUp(false)
		, TotalActiveParticles(0)
		, NumSignificantEmitters(0)
		, MinDrawDistance(0.0f)
		, MaxDrawDistance(0.0f)
	{
		bTickInEditor = true;
		RandStream.Initalize(Time::Cycles());

		bStatic = false;
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

		SpawnEvents.clear();
		DeathEvents.clear();
		CollisionEvents.clear();
		BurstEvents.clear();
		KismetEvents.clear();

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
		{
			int32 EmitterIndex;
			// Now, process any events that have occurred.
			for (EmitterIndex = 0; EmitterIndex < Emitters.size(); EmitterIndex++)
			{
				ParticleEmitterInstance* Instance = Emitters[EmitterIndex];
				if (Instance && Instance->bEnabled)
				{
					if (EmitterIndex + 1 < Emitters.size())
					{
						ParticleEmitterInstance* NextInstance = Emitters[EmitterIndex+1];
						ApplicationMisc::Prefetch(NextInstance);
					}
		
					if (Instance->Emitter && Instance->Emitter->IsEnabled())
					{
						Instance->ProcessParticleEvents(DeltaTime, bSuppressSpawning);
					}
				}
			}
		}

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
		PrimitiveComponent::Serialize(Ar);

		if ( Ar.IsLoading() )
		{
			std::string TemplatePath = "";
			Ar >> TemplatePath;
			SetTemplate(AssetHandle<ParticleSystem>(TemplatePath));

			{
				uint8 FloatParamtersCount;
				Ar >> FloatParamtersCount;
				FloatParams.resize(FloatParamtersCount);
				for (int32 Index = 0; Index < FloatParamtersCount; Index++)
				{
					FloatParams[Index].Serialize(Ar);
				}
			}

			{
				uint8 VectorParamtersCount;
				Ar >> VectorParamtersCount;
				VectorParams.resize(VectorParamtersCount);
				for (int32 Index = 0; Index < VectorParamtersCount; Index++)
				{
					VectorParams[Index].Serialize(Ar);
				}
			}

			Ar >> MinDrawDistance;
			Ar >> MaxDrawDistance;

			drn_check(!bStatic); // this is always dynamic
		}

		else
		{
			Ar << Template.GetPath();

			{
				const uint8 FloatParamtersCount = FloatParams.size();
				Ar << FloatParamtersCount;
				for (int32 Index = 0; Index < FloatParamtersCount; Index++)
				{
					FloatParams[Index].Serialize(Ar);
				}
			}

			{
				const uint8 VectorParamtersCount = VectorParams.size();
				Ar << VectorParamtersCount;
				for (int32 Index = 0; Index < VectorParamtersCount; Index++)
				{
					VectorParams[Index].Serialize(Ar);
				}
			}

			Ar << MinDrawDistance;
			Ar << MaxDrawDistance;
		}

	}

	void ParticleSystemComponent::RegisterComponent( World* InOwningWorld )
	{
		PrimitiveComponent::RegisterComponent(InOwningWorld);

		RegisterSceneProxies();

#if WITH_EDITOR
		AssetHandle<Texture2D> DefaultIcon( "Engine\\Content\\EditorResources\\ComponentIcons\\T_ParticleIcon.drn" );
		DefaultIcon.Load();
		
		m_Sprite->SetSprite( DefaultIcon );
#endif
	}

	void ParticleSystemComponent::UnRegisterComponent()
	{
		UnregisterSceneProxies();

		PrimitiveComponent::UnRegisterComponent();
	}

	void ParticleSystemComponent::RegisterSceneProxies()
	{
		for (ParticleEmitterInstance* Instance : Emitters)
		{
			if (Instance)
			{
				Instance->RegisterSceneProxy();
			}
		}
	}

	void ParticleSystemComponent::UnregisterSceneProxies()
	{
		for (ParticleEmitterInstance* Instance : Emitters)
		{
			if (Instance)
			{
				Instance->UnregisterSceneProxy();
			}
		}
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
						drn_check(Emitter->EmitterType);
						EEmitterType EType = Emitter->GetEmitterType()->GetType();

						if (EType == EEmitterType::Mesh)
						{
							TRefCountPtr<ParticleMeshEmitterInstance> MeshEmitter = new ParticleMeshEmitterInstance();
							Instance = Emitters[i] = MeshEmitter;
						}
						else if (EType == EEmitterType::Sprite_Cpu)
						{
							TRefCountPtr<ParticleCpuSpriteEmitterInstance> SpriteEmitter = new ParticleCpuSpriteEmitterInstance();
							Instance = Emitters[i] = SpriteEmitter;
						}
						else
						{
							drn_check(false);
						}
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

			if (Template->WarmupTime > 0.0f)
			{
				//bool bSaveSkipUpdate = bSkipUpdateDynamicDataDuringTick;
				//bSkipUpdateDynamicDataDuringTick = true;
				bWarmingUp = true;
				for (int32 i=0; i<Emitters.size(); i++)
				{
					if (Emitters[i])
					{
						Emitters[i]->ResetBurstList();
					}
				}

				float WarmupElapsed = 0.f;
				float WarmupTimestep = 0.032f;
				if (Template->WarmupTickRate > 0)
				{
					WarmupTimestep = (Template->WarmupTickRate <= Template->WarmupTime) ? Template->WarmupTickRate : Template->WarmupTime;
				}

				while (WarmupElapsed < Template->WarmupTime)
				{
					Tick(WarmupTimestep);
					WarmupElapsed += WarmupTimestep;
				}

				bWarmingUp = false;
				//bSkipUpdateDynamicDataDuringTick = bSaveSkipUpdate;
			}
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

	void ParticleSystemComponent::SetFloatParameter( const std::string& InName, const ParticleSysParamFloat& InParam )
	{
		if(InName.empty())
		{
			return;
		}

		for (int32 i = 0; i < FloatParams.size(); i++)
		{
			ParticleSysParamFloat& Param = FloatParams[i];
			if (Param.Name == InName)
			{
				Param.bUseLowRange	= InParam.bUseLowRange;
				Param.Scalar		= InParam.Scalar;
				Param.Scalar_Low	= InParam.Scalar_Low;
				return;
			}
		}

		FloatParams.push_back(InParam);
	}

	bool ParticleSystemComponent::GetFloatParameter( const std::string& InName, float& OutFloat )
	{
		if(InName.empty())
		{
			return false;
		}

		for (int32 i = 0; i < FloatParams.size(); i++)
		{
			const ParticleSysParamFloat& Param = FloatParams[i];
			if (Param.Name == InName)
			{
				if (Param.bUseLowRange)
				{
					OutFloat = Param.Scalar + (Param.Scalar_Low - Param.Scalar) * RandStream.GetFraction();
					return true;
				}

				else
				{
					OutFloat = Param.Scalar;
					return true;
				}
			}
		}

		return false;
	}

	void ParticleSystemComponent::SetVectorParameter( const std::string& InName, const ParticleSysParamVector& InParam )
	{
		if(InName.empty())
		{
			return;
		}

		for (int32 i = 0; i < VectorParams.size(); i++)
		{
			ParticleSysParamVector& Param = VectorParams[i];
			if (Param.Name == InName)
			{
				Param.bUseLowRange	= InParam.bUseLowRange;
				Param.Value			= InParam.Value;
				Param.Value_Low		= InParam.Value_Low;
				return;
			}
		}

		VectorParams.push_back(InParam);
	}

	bool ParticleSystemComponent::GetVectorParameter( const std::string& InName, Vector& OutVector )
	{
		if(InName.empty())
		{
			return false;
		}

		for (int32 i = 0; i < VectorParams.size(); i++)
		{
			const ParticleSysParamVector& Param = VectorParams[i];
			if (Param.Name == InName)
			{
				if (Param.bUseLowRange)
				{
					OutVector = Param.Value + (Param.Value_Low - Param.Value) * RandStream.GetFraction();
					return true;
				}

				else
				{
					OutVector = Param.Value;
					return true;
				}
			}
		}

		return false;
	}

	void ParticleSystemComponent::ReportEventSpawn( const std::string& InEventName, const float InEmitterTime, const Vector& InLocation,
		const Vector& InVelocity /*, const TArray<class UParticleModuleEventSendToGame*>& InEventData*/ )
	{
		SpawnEvents.push_back({});
		ParticleEventSpawnData* SpawnData = &SpawnEvents.back();
		SpawnData->Type = EPET_Spawn;
		SpawnData->EventName = InEventName;
		SpawnData->EmitterTime = InEmitterTime;
		SpawnData->Location = InLocation;
		SpawnData->Velocity = InVelocity;
		//SpawnData->EventData = InEventData;
	}

	void ParticleSystemComponent::ReportEventDeath( const std::string& InEventName, const float InEmitterTime, const Vector& InLocation,
		const Vector& InVelocity /*, const TArray<class UParticleModuleEventSendToGame*>& InEventData*/, const float   InParticleTime )
	{
		DeathEvents.push_back({});
		ParticleEventDeathData* DeathData = &DeathEvents.back();
		DeathData->Type = EPET_Death;
		DeathData->EventName = InEventName;
		DeathData->EmitterTime = InEmitterTime;
		DeathData->Location = InLocation;
		DeathData->Velocity = InVelocity;
		//DeathData->EventData = InEventData;
		DeathData->ParticleTime = InParticleTime;
	}

	void ParticleSystemComponent::ReportEventCollision( const std::string& InEventName, const float InEmitterTime, const Vector& InLocation,
		const Vector& InDirection, const Vector& InVelocity /*, const TArray<class UParticleModuleEventSendToGame*>& InEventData*/,
		const float InParticleTime, const Vector& InNormal, const float InTime, const int32 InItem, const std::string& InBoneName, class PhysicalMaterial* PhysMat )
	{
		CollisionEvents.push_back({});
		ParticleEventCollideData* CollideData = &CollisionEvents.back();
		CollideData->Type = EPET_Collision;
		CollideData->EventName = InEventName;
		CollideData->EmitterTime = InEmitterTime;
		CollideData->Location = InLocation;
		CollideData->Direction = InDirection;
		CollideData->Velocity = InVelocity;
		//CollideData->EventData = InEventData;
		CollideData->ParticleTime = InParticleTime;
		CollideData->Normal = InNormal;
		CollideData->Time = InTime;
		CollideData->Item = InItem;
		CollideData->BoneName = InBoneName;
		CollideData->PhysMat = PhysMat;
	}

	void ParticleSystemComponent::ReportEventBurst( const std::string& InEventName, const float InEmitterTime, const int32 InParticleCount,
		const Vector& InLocation /*, const TArray<class UParticleModuleEventSendToGame*>& InEventData*/ )
	{
		BurstEvents.push_back({});
		ParticleEventBurstData* BurstData = &BurstEvents.back();
		BurstData->Type = EPET_Burst;
		BurstData->EventName = InEventName;
		BurstData->EmitterTime = InEmitterTime;
		BurstData->ParticleCount = InParticleCount;
		BurstData->Location = InLocation;
		//BurstData->EventData = InEventData;
	}

	void ParticleSystemComponent::GenerateParticleEvent( const std::string& InEventName, const float InEmitterTime, const Vector& InLocation, const Vector& InDirection, const Vector& InVelocity )
	{
		KismetEvents.push_back({});
		ParticleEventKismetData* KismetData = &KismetEvents.back();
		KismetData->Type = EPET_Blueprint;
		KismetData->EventName = InEventName;
		KismetData->EmitterTime = InEmitterTime;
		KismetData->Location = InLocation;
		KismetData->Velocity = InVelocity;
	}

	bool ParticleSystemComponent::ParticleLineCheck( HitResult& Hit, Actor* SourceActor, const Vector& End, const Vector& Start, const Vector& HalfExtent, const std::vector<ECollisionChannel>& ObjectTypes )
	{
		drn_check(GetWorld());
		if ( HalfExtent.IsZero() )
		{
			CollisionQueryParams QueryParams(SourceActor);
			QueryParams.bReturnPhysicalMaterial = true;
			return GetWorld()->GetPhysicScene()->RaycastSingle(GetWorld(), Hit, Start, End, QueryParams, ObjectTypes);
		}
		else
		{
			CollisionQueryParams BoxParams(SourceActor);
			BoxParams.bReturnPhysicalMaterial = true;
			return GetWorld()->GetPhysicScene()->GeomSweepSingle(GetWorld(), Hit, PxBoxGeometry(), Start, End, Quat::Identity, BoxParams, ObjectTypes);
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
		}

		if (ImGui::InputFloat("MinDrawDistance", &MinDrawDistance))
		{
			//SetMinDrawDistance(MinDrawDistance);
		}

		if (ImGui::InputFloat("MaxDrawDistance", &MaxDrawDistance))
		{
			//SetMaxDrawDistance(MaxDrawDistance);
		}

		if (ImGui::CollapsingHeader("Float Parameters", ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::PushID("Float Params");

			if (ImGui::Button("Add"))
			{
				FloatParams.push_back( {} );
			}

			if (ImGui::Button("Clear"))
			{
				FloatParams.clear();
			}

			for (int32 i = 0; i < FloatParams.size(); i++)
			{
				ImGui::PushID(i);

				FloatParams[i].Draw();
				ImGui::Separator();

				ImGui::PopID();
			}

			ImGui::PopID();
		}

		if (ImGui::CollapsingHeader("Vector Parameters", ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::PushID("Vector Params");

			if (ImGui::Button("Add"))
			{
				VectorParams.push_back( {} );
			}

			if (ImGui::Button("Clear"))
			{
				VectorParams.clear();
			}

			for (int32 i = 0; i < VectorParams.size(); i++)
			{
				ImGui::PushID(i);

				VectorParams[i].Draw();
				ImGui::Separator();

				ImGui::PopID();
			}

			ImGui::PopID();
		}
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

	void ParticleSystemComponent::SetSelectedInEditor( bool SelectedInEditor )
	{
		PrimitiveComponent::SetSelectedInEditor(SelectedInEditor);

		for (ParticleEmitterInstance* Instance : Emitters)
		{
			if (Instance && Instance->SceneProxy)
			{
				Instance->SceneProxy->SetSelectedInEditor(SelectedInEditor);
			}
		}
	}

	void ParticleSystemComponent::SetSelectable( bool Selectable )
	{
		PrimitiveComponent::SetSelectable(Selectable);

		for (ParticleEmitterInstance* Instance : Emitters)
		{
			if (Instance && Instance->SceneProxy)
			{
				Instance->SceneProxy->SetSelectable(Selectable);
			}
		}
	}

#endif

// ------------------------------------------------------------------------------------------

	void ParticleSysParam::Serialize( Archive& Ar )
	{
		if (Ar.IsLoading())
		{
			Ar >> Name;
		}
		else
		{
			Ar << Name;
		}
	}

#if WITH_EDITOR
	bool ParticleSysParam::Draw()
	{
		bool bDirty = false;

		const int32 TextCharLimit = 64;
		char InputText[TextCharLimit];
		strcpy_s(InputText, sizeof(InputText), Name.c_str());

		if ( ImGui::InputText( "Parameter Name", InputText, TextCharLimit ) )
		{
			Name = InputText;
			bDirty = true;
		}

		return bDirty;
	}
#endif

	void ParticleSysParamFloat::Serialize( Archive& Ar )
	{
		ParticleSysParam::Serialize(Ar);

		if (Ar.IsLoading())
		{
			Ar >> bUseLowRange;
			Ar >> Scalar;
			Ar >> Scalar_Low;
		}
		else
		{
			Ar << bUseLowRange;
			Ar << Scalar;
			Ar << Scalar_Low;
		}
	}

#if WITH_EDITOR
	bool ParticleSysParamFloat::Draw()
	{
		bool bDirty = ParticleSysParam::Draw();

		bDirty |= ImGui::Checkbox("Use Low Range", &bUseLowRange);
		bDirty |= ImGui::InputFloat("Scalar", &Scalar);
		bDirty |= ImGui::InputFloat("Scalar Low", &Scalar_Low);

		return bDirty;
	}
#endif

	void ParticleSysParamVector::Serialize( Archive& Ar )
	{
		ParticleSysParam::Serialize(Ar);

		if (Ar.IsLoading())
		{
			Ar >> bUseLowRange;
			Ar >> Value;
			Ar >> Value_Low;
		}
		else
		{
			Ar << bUseLowRange;
			Ar << Value;
			Ar << Value_Low;
		}
	}

#if WITH_EDITOR
	bool ParticleSysParamVector::Draw()
	{
		bool bDirty = ParticleSysParam::Draw();

		bDirty |= ImGui::Checkbox("Use Low Range", &bUseLowRange);
		bDirty |= Value.Draw("Vector", "Vector");
		bDirty |= Value_Low.Draw("Vector Low", "Vector Low");

		return bDirty;
	}
#endif

}  // namespace Drn