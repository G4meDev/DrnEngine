#include "DrnPCH.h"
#include "EngineTypes.h"

#include "Editor/Misc/EditorMisc.h"
#include "Runtime/Tool/ScenePointCloudImporter.h"

#if WITH_EDITOR
#include "imgui.h"
#endif

namespace Drn
{
	CollisionResponseContainer CollisionResponseContainer::DefaultResponseContainer( ECR_Block );
	CollisionProfile* CollisionProfile::SingletonInstance = nullptr;

	EngineTypes* EngineTypes::m_SingletonInstance;

	void RigidBodyCollisionInfo::SetFrom( const BodyInstance* BodyInst )
	{
		if (BodyInst)
		{
			m_Component = BodyInst->GetOwnerComponent();
			m_Actor = m_Component->GetOwningActor();
		}
		else
		{
			m_Component = nullptr;
			m_Actor = nullptr;
		}
	}

	BodyInstance* RigidBodyCollisionInfo::GetBodyInstance() const
	{
		BodyInstance* Result = nullptr;
		
		if (m_Component)
		{
			Result = &m_Component->GetBodyInstance();
		}

		return Result;
	}

	void CollisionImpactData::SwapContactOrders()
	{
		for ( RigidBodyContactInfo& Info : ContactInfos )
		{
			Info.SwapOrder();
		}
	}

	void RigidBodyContactInfo::SwapOrder()
	{
		ContactNormal = ContactNormal * -1;
	}


	void EngineTypes::RegisterSerializableActor( EActorType ActorType, std::function<Actor*( World* InWorld, Archive& Ar )>&& Func )
	{
		EngineTypes::Get()->m_ActorSerializationMap[ActorType] = Func;
	}

	void EngineTypes::Register()
	{
		REGISTER_SERIALIZABLE_ACTOR( EActorType::StaticMeshActor			, StaticMeshActor );
		REGISTER_SERIALIZABLE_ACTOR( EActorType::InstancedStaticMeshActor	, InstancedStaticMeshActor );
		REGISTER_SERIALIZABLE_ACTOR( EActorType::PointLight					, PointLightActor );
		REGISTER_SERIALIZABLE_ACTOR( EActorType::SpotLight					, SpotLightActor );
		REGISTER_SERIALIZABLE_ACTOR( EActorType::DirectionalLight			, DirectionalLightActor );
		REGISTER_SERIALIZABLE_ACTOR( EActorType::SkyLight					, SkyLightActor );
		REGISTER_SERIALIZABLE_ACTOR( EActorType::PostProcessVolume			, PostProcessVolume );
		REGISTER_SERIALIZABLE_ACTOR( EActorType::DecalActor					, DecalActor );
		REGISTER_SERIALIZABLE_ACTOR( EActorType::Pawn						, Pawn );
		REGISTER_SERIALIZABLE_ACTOR( EActorType::Character					, Character );
		REGISTER_SERIALIZABLE_ACTOR( EActorType::SphereReflectionCapture	, SphereReflectionCapture );
		REGISTER_SERIALIZABLE_ACTOR( EActorType::WheeledVehiclePawn			, WheeledVehiclePawn );
		REGISTER_SERIALIZABLE_ACTOR( EActorType::ScenePointCloudImporter	, ScenePointCloudImporter );

		CollisionChannelDisplayNames[ECC_WorldStatic]	= "WorldStatic";
		CollisionChannelDisplayNames[ECC_WorldDynamic]	= "WorldDynamic";
		CollisionChannelDisplayNames[ECC_Pawn]			= "Pawn";
		CollisionChannelDisplayNames[ECC_Visibility]	= "Visibility";
		CollisionChannelDisplayNames[ECC_Camera]		= "Camera";
		CollisionChannelDisplayNames[ECC_PhysicsBody]	= "PhysicsBody";
		CollisionChannelDisplayNames[ECC_Vehicle]		= "Vehicle";
		CollisionChannelDisplayNames[ECC_Destructible]	= "Destructible";

		CollisionProfile::Get()->ProfileNames.push_back(CUSTOM_COLLISION_PROFILE_NAME);

		CollisionResponseTemplate NoCollisionProfile("NoCollision", ECollisionEnabled::NoCollision, ECC_WorldStatic);
		NoCollisionProfile.ResponseToChannels.SetAllChannels(ECR_Ignore);
		CollisionProfile::Get()->AddProfile(NoCollisionProfile);

		CollisionResponseTemplate BlockAllProfile("BlockAll", ECollisionEnabled::QueryAndPhysics, ECC_WorldStatic);
		BlockAllProfile.ResponseToChannels.SetAllChannels(ECR_Block);
		CollisionProfile::Get()->AddProfile(BlockAllProfile);

		CollisionResponseTemplate OverlapAllProfile("OverlapAll", ECollisionEnabled::QueryOnly, ECC_WorldStatic);
		OverlapAllProfile.ResponseToChannels.SetAllChannels(ECR_Overlap);
		CollisionProfile::Get()->AddProfile(OverlapAllProfile);

		CollisionResponseTemplate BlockAllDynamicProfile("BlockAllDynamic", ECollisionEnabled::QueryAndPhysics, ECC_WorldDynamic);
		BlockAllDynamicProfile.ResponseToChannels.SetAllChannels(ECR_Block);
		CollisionProfile::Get()->AddProfile(BlockAllDynamicProfile);

		CollisionResponseTemplate OverlapAllDynamicProfile("OverlapAllDynamic", ECollisionEnabled::QueryOnly, ECC_WorldDynamic);
		OverlapAllDynamicProfile.ResponseToChannels.SetAllChannels(ECR_Overlap);
		CollisionProfile::Get()->AddProfile(OverlapAllDynamicProfile);

		CollisionResponseTemplate IgnoreOnlyPawnProfile("IgnoreOnlyPawn", ECollisionEnabled::QueryOnly, ECC_WorldDynamic);
		IgnoreOnlyPawnProfile.ResponseToChannels.SetAllChannels(ECR_Block);
		IgnoreOnlyPawnProfile.ResponseToChannels.SetResponse(ECollisionChannel::ECC_Pawn, ECR_Ignore);
		IgnoreOnlyPawnProfile.ResponseToChannels.SetResponse(ECollisionChannel::ECC_Vehicle, ECR_Ignore);
		CollisionProfile::Get()->AddProfile(IgnoreOnlyPawnProfile);

		CollisionResponseTemplate OverlapOnlyPawnProfile("OverlapOnlyPawn", ECollisionEnabled::QueryOnly, ECC_WorldDynamic);
		OverlapOnlyPawnProfile.ResponseToChannels.SetAllChannels(ECR_Block);
		OverlapOnlyPawnProfile.ResponseToChannels.SetResponse(ECollisionChannel::ECC_Camera, ECR_Ignore);
		OverlapOnlyPawnProfile.ResponseToChannels.SetResponse(ECollisionChannel::ECC_Pawn, ECR_Overlap);
		OverlapOnlyPawnProfile.ResponseToChannels.SetResponse(ECollisionChannel::ECC_Vehicle, ECR_Overlap);
		CollisionProfile::Get()->AddProfile(OverlapOnlyPawnProfile);

		CollisionResponseTemplate PawnProfile("Pawn", ECollisionEnabled::QueryAndPhysics, ECC_Pawn);
		PawnProfile.ResponseToChannels.SetAllChannels(ECR_Block);
		PawnProfile.ResponseToChannels.SetResponse(ECollisionChannel::ECC_Visibility, ECR_Ignore);
		CollisionProfile::Get()->AddProfile(PawnProfile);

		CollisionResponseTemplate SpectatorProfile("Spectator", ECollisionEnabled::QueryOnly, ECC_Pawn);
		SpectatorProfile.ResponseToChannels.SetAllChannels(ECR_Ignore);
		SpectatorProfile.ResponseToChannels.SetResponse(ECollisionChannel::ECC_WorldStatic, ECR_Block);
		CollisionProfile::Get()->AddProfile(SpectatorProfile);

		CollisionResponseTemplate CharacterMeshProfile("CharacterMesh", ECollisionEnabled::QueryOnly, ECC_Pawn);
		CharacterMeshProfile.ResponseToChannels.SetAllChannels(ECR_Block);
		CharacterMeshProfile.ResponseToChannels.SetResponse(ECollisionChannel::ECC_Pawn, ECR_Ignore);
		CharacterMeshProfile.ResponseToChannels.SetResponse(ECollisionChannel::ECC_Vehicle, ECR_Ignore);
		CollisionProfile::Get()->AddProfile(CharacterMeshProfile);

		CollisionResponseTemplate PhysicsActorProfile("PhysicsActor", ECollisionEnabled::QueryAndPhysics, ECC_PhysicsBody);
		PhysicsActorProfile.ResponseToChannels.SetAllChannels(ECR_Block);
		CollisionProfile::Get()->AddProfile(PhysicsActorProfile);

		CollisionResponseTemplate DestructibleProfile("DestructibleActor", ECollisionEnabled::QueryAndPhysics, ECC_Destructible);
		DestructibleProfile.ResponseToChannels.SetAllChannels(ECR_Block);
		CollisionProfile::Get()->AddProfile(DestructibleProfile);

		CollisionResponseTemplate InvisibleWallProfile("InvisibleWall", ECollisionEnabled::QueryAndPhysics, ECC_WorldStatic);
		InvisibleWallProfile.ResponseToChannels.SetAllChannels(ECR_Block);
		InvisibleWallProfile.ResponseToChannels.SetResponse(ECollisionChannel::ECC_Visibility, ECR_Ignore);
		CollisionProfile::Get()->AddProfile(InvisibleWallProfile);

		CollisionResponseTemplate InvisibleWallDynamicProfile("InvisibleWallDynamic", ECollisionEnabled::QueryAndPhysics, ECC_WorldDynamic);
		InvisibleWallDynamicProfile.ResponseToChannels.SetAllChannels(ECR_Block);
		InvisibleWallDynamicProfile.ResponseToChannels.SetResponse(ECollisionChannel::ECC_Visibility, ECR_Ignore);
		CollisionProfile::Get()->AddProfile(InvisibleWallDynamicProfile);

		CollisionResponseTemplate TriggerProfile("Trigger", ECollisionEnabled::QueryOnly, ECC_WorldDynamic);
		TriggerProfile.ResponseToChannels.SetAllChannels(ECR_Overlap);
		TriggerProfile.ResponseToChannels.SetResponse(ECollisionChannel::ECC_Visibility, ECR_Ignore);
		CollisionProfile::Get()->AddProfile(TriggerProfile);

		CollisionResponseTemplate RagdollProfile("Ragdoll", ECollisionEnabled::QueryAndPhysics, ECC_PhysicsBody);
		RagdollProfile.ResponseToChannels.SetAllChannels(ECR_Block);
		RagdollProfile.ResponseToChannels.SetResponse(ECollisionChannel::ECC_Visibility, ECR_Ignore);
		RagdollProfile.ResponseToChannels.SetResponse(ECollisionChannel::ECC_Pawn, ECR_Ignore);
		CollisionProfile::Get()->AddProfile(RagdollProfile);

		CollisionResponseTemplate VehicleProfile("Vehicle", ECollisionEnabled::QueryAndPhysics, ECC_Vehicle);
		VehicleProfile.ResponseToChannels.SetAllChannels(ECR_Block);
		CollisionProfile::Get()->AddProfile(VehicleProfile);

		CollisionResponseTemplate UIProfile("UI", ECollisionEnabled::QueryOnly, ECC_WorldDynamic);
		UIProfile.ResponseToChannels.SetAllChannels(ECR_Overlap);
		UIProfile.ResponseToChannels.SetResponse(ECollisionChannel::ECC_Visibility, ECR_Block);
		CollisionProfile::Get()->AddProfile(UIProfile);

		RegisterGameTypes();
	}

	EngineTypes* EngineTypes::Get()
	{
		if ( !m_SingletonInstance )
		{
			m_SingletonInstance = new EngineTypes();
		}
		return m_SingletonInstance;
	}

	void StaticShadowDepthMapData::Empty()
	{
		ShadowMapSizeX = 0;
		ShadowMapSizeY = 0;
		DepthSamples.clear();
	}

	Archive& operator<<( Archive& Ar, StaticShadowDepthMapData& ShadowMap )
	{
		//Ar << ShadowMap.WorldToLight;
		Ar << ShadowMap.ShadowMapSizeX;
		Ar << ShadowMap.ShadowMapSizeY;

		uint32 DepthSampleCount = ShadowMap.DepthSamples.size();
		Ar << DepthSampleCount;

		for (uint32 i = 0; i < DepthSampleCount; i++)
		{
			Ar << ShadowMap.DepthSamples[i];
		}

		return Ar;
	}

	Archive& operator>>(Archive& Ar, StaticShadowDepthMapData& ShadowMap)
	{
		//Ar >> ShadowMap.WorldToLight;
		Ar >> ShadowMap.ShadowMapSizeX;
		Ar >> ShadowMap.ShadowMapSizeY;

		uint32 DepthSampleCount;
		Ar >> DepthSampleCount;
		ShadowMap.DepthSamples.resize(DepthSampleCount);

		for (uint32 i = 0; i < DepthSampleCount; i++)
		{
			Ar >> ShadowMap.DepthSamples[i];
		}

		return Ar;
	}

	void ReflectionCaptureData::Empty()
	{
		CubemapSize = 0;
		SamplesData.clear();
	}

	Archive& operator<<( Archive& Ar, ReflectionCaptureData& CaptureData )
	{
		Ar << CaptureData.CubemapSize;

		uint32 SamplesDataCount = CaptureData.SamplesData.size();
		Ar << SamplesDataCount;

		for (uint32 i = 0; i < SamplesDataCount; i++)
		{
			Ar << CaptureData.SamplesData[i];
		}

		return Ar;
	}

	Archive& operator>>(Archive& Ar, ReflectionCaptureData& CaptureData)
	{
		Ar >> CaptureData.CubemapSize;

		uint32 SamplesDataCount;
		Ar >> SamplesDataCount;
		CaptureData.SamplesData.resize(SamplesDataCount);

		for (uint32 i = 0; i < SamplesDataCount; i++)
		{
			Ar >> CaptureData.SamplesData[i];
		}

		return Ar;
	}

// --------------------------------------------------------------------------------

	CollisionResponseContainer::CollisionResponseContainer()
	{
		*this = CollisionResponseContainer::DefaultResponseContainer;
	}

	CollisionResponseContainer::CollisionResponseContainer( ECollisionResponse DefaultResponse )
	{
		SetAllChannels(DefaultResponse);
	}

	bool CollisionResponseContainer::SetResponse( ECollisionChannel Channel, ECollisionResponse NewResponse )
	{
		if (Channel < _countof(EnumArray))
		{
			uint8& CurrentResponse = EnumArray[Channel];
			if (CurrentResponse != NewResponse)
			{
				CurrentResponse = NewResponse;
				return true;
			}
		}
		return false;
	}

	bool CollisionResponseContainer::SetAllChannels( ECollisionResponse NewResponse )
	{
		bool bHasChanged = false;
		for(int32 i=0; i < _countof(EnumArray); i++)
		{
			uint8& CurrentResponse = EnumArray[i];
			if (CurrentResponse != NewResponse)
			{
				CurrentResponse = NewResponse;
				bHasChanged = true;
			}
		}
		return bHasChanged;
	}

	bool CollisionResponseContainer::ReplaceChannels( ECollisionResponse OldResponse, ECollisionResponse NewResponse )
	{
		bool bHasChanged = false;
		for (int32 i = 0; i < _countof(EnumArray); i++)
		{
			uint8& CurrentResponse = EnumArray[i];
			if(CurrentResponse == OldResponse)
			{
				CurrentResponse = NewResponse;
				bHasChanged = true;
			}
		}
		return bHasChanged;
	}

	CollisionResponseContainer CollisionResponseContainer::CreateMinContainer( const CollisionResponseContainer& A, const CollisionResponseContainer& B )
	{
		CollisionResponseContainer Result;
		for(int32 i=0; i < _countof(Result.EnumArray); i++)
		{
			Result.EnumArray[i] = std::min(A.EnumArray[i], B.EnumArray[i]);
		}
		return Result;
	}

	Archive& operator<<( Archive& Ar, CollisionResponseContainer& Container )
	{
		uint64 PackedData = 0;
		for (int32 i = 0; i < 32; i++)
		{
			PackedData |= ((uint64)Container.EnumArray[i] & 0x0003) << (i * 2);
		}

		Ar << PackedData;
		return Ar;
	}

	Archive& operator>>( Archive& Ar, CollisionResponseContainer& Container )
	{
		uint64 PackedData = 0;
		Ar >> PackedData;

		for (int32 i = 0; i < 32; i++)
		{
			Container.EnumArray[i] = (PackedData >> (i * 2)) & 0x0003;
		}

		return Ar;
	}

	CollisionResponseTemplate::CollisionResponseTemplate()
		: Name("NoName")
		, CollisionEnabled(ECollisionEnabled::NoCollision)
		, ObjectType(ECollisionChannel::ECC_WorldStatic)
		, ResponseToChannels()
	{}

	CollisionResponseTemplate::CollisionResponseTemplate( const std::string&& InName, ECollisionEnabled InCollisionEnabled, ECollisionChannel InObjectType )
		: Name(InName)
		, CollisionEnabled(InCollisionEnabled)
		, ObjectType(InObjectType)
		, ResponseToChannels()
	{}

	CollisionProfile* CollisionProfile::Get()
	{
		if (!SingletonInstance)
		{
			SingletonInstance = new CollisionProfile();
		}

		return SingletonInstance;
	}

	bool CollisionProfile::GetProfile( const std::string& ProfileName, CollisionResponseTemplate& ProfileData ) const
	{
		auto It = Profiles.find(ProfileName);
		if (It == Profiles.end())
		{
			drn_check(true);
			return false;
		}

		ProfileData = It->second;
		return true;
	}

	void CollisionProfile::AddProfile( const CollisionResponseTemplate& ProfileData )
	{
		auto It = Profiles.find(ProfileData.Name);
		drn_check(It == Profiles.end());

		Profiles[ProfileData.Name] = ProfileData;
		ProfileNames.push_back(ProfileData.Name);
	}

	bool CollisionProfile::UpdateProfileChannelResponse( const std::string& ProfileName, ECollisionChannel CollisionChannel, ECollisionResponse CollisionResponse )
	{
		auto It = Profiles.find(ProfileName);
		if (It == Profiles.end())
		{
			drn_check(true);
			return false;
		}

		It->second.ResponseToChannels.SetResponse(CollisionChannel, CollisionResponse);
		return true;
	}

	PhysicsFilterBuilder::PhysicsFilterBuilder( ECollisionChannel InObjectType, uint8 MaskFilter, const CollisionResponseContainer& ResponseToChannels )
		: BlockingBits(0)
		, TouchingBits(0)
		, Word3(0)
	{
		for (int32 i = 0; i < _countof(ResponseToChannels.EnumArray); i++)
		{
			if (ResponseToChannels.EnumArray[i] == ECR_Block)
			{
				const uint32 ChannelBit = ( 1 << i );
				BlockingBits |= ChannelBit;
			}
			else if (ResponseToChannels.EnumArray[i] == ECR_Overlap)
			{
				const uint32 ChannelBit = ( 1 << i );
				TouchingBits |= ChannelBit;
			}
		}

		Word3 = CreateChannelAndFilter(InObjectType, MaskFilter);
	}

	void CreateShapeFilterData( const uint8 MyChannel, const uint8 MaskFilter, const int32 ActorID, const CollisionResponseContainer& ResponseToChannels, uint32 ComponentID,
		uint16 BodyIndex, CollisionFilterData& OutQueryData, CollisionFilterData& OutSimData, bool bEnableCCD, bool bEnableContactNotify,
		bool bStaticShape, bool bModifyContacts )
	{
		PhysicsFilterBuilder Builder((ECollisionChannel)MyChannel, MaskFilter, ResponseToChannels);
		Builder.ConditionalSetFlags(EPDF_CCD, bEnableCCD);
		Builder.ConditionalSetFlags(EPDF_ContactNotify, bEnableContactNotify);
		Builder.ConditionalSetFlags(EPDF_StaticShape, bStaticShape);
		Builder.ConditionalSetFlags(EPDF_ModifyContacts, bModifyContacts);

		OutQueryData = CollisionFilterData();
		OutSimData = CollisionFilterData();
		Builder.GetQueryData(ActorID, OutQueryData.Word0, OutQueryData.Word1, OutQueryData.Word2, OutQueryData.Word3);
		Builder.GetSimData(BodyIndex, ComponentID, OutSimData.Word0, OutSimData.Word1, OutSimData.Word2, OutSimData.Word3);
	}

#if WITH_EDITOR

	bool DrawCollisionProfile( const std::string& Label, std::string& ProfileName )
	{
		auto& ProfileNames = CollisionProfile::Get()->ProfileNames;
		auto It = std::find(ProfileNames.begin(), ProfileNames.end(), ProfileName);
		int32 CurrentProfileIndex = It == ProfileNames.end() ? 0 : std::distance(ProfileNames.begin(), It);

		int32 SelectedIndex = CurrentProfileIndex;
		if (ImGui::BeginCombo(Label.c_str(), ProfileNames[CurrentProfileIndex].c_str()))
		{
			for (int32 i = 0; i < ProfileNames.size(); i++)
			{
				const bool bSelected = i == CurrentProfileIndex;
				if (ImGui::Selectable(ProfileNames[i].c_str(), bSelected))
				{
					SelectedIndex = i;
				}

				if (bSelected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}

			ImGui::EndCombo();

			if (CurrentProfileIndex != SelectedIndex)
			{
				ProfileName = ProfileNames[SelectedIndex];
				return true;
			}
		}

		return false;
	}

	bool DrawCollisionObjectType( const std::string& Label, ECollisionChannel& CollisionChannel )
	{
		auto& CollisionChannelDisplayNames = EngineTypes::Get()->CollisionChannelDisplayNames;

		if ( ImGui::BeginCombo( Label.c_str(), CollisionChannelDisplayNames[CollisionChannel].c_str() ) )
		{
			ECollisionChannel SelectedChannel = CollisionChannel;

			for (int32 i = 0; i < 32; i++)
			{
				// skip unused channels
				if (CollisionChannelDisplayNames[i] == "")
				{
					continue;
				}

				const bool bSelected = i == CollisionChannel;
				if (ImGui::Selectable(CollisionChannelDisplayNames[i].c_str(), bSelected))
				{
					SelectedChannel = static_cast<ECollisionChannel>(i);
				}

				if (bSelected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}

			ImGui::EndCombo();

			if (CollisionChannel != SelectedChannel)
			{
				CollisionChannel = SelectedChannel;
				return true;
			}
		}

		return false;
	}

	bool DrawCollisionEnabled( const std::string& Label, ECollisionEnabled& CollisionEnabled )
	{
		const char* Options[] = { "NoCollision", "QueryOnly", "PhysicsOnly", "QueryAndPhysics" };

		int32 Selected = (uint32)CollisionEnabled;
		if ( ImGui::Combo(Label.c_str(), &Selected, Options, IM_ARRAYSIZE(Options)) )
		{
			CollisionEnabled = (ECollisionEnabled)Selected;
			return true;
		}

		return false;
	}

	bool CollisionResponseContainer::Draw()
	{
		bool bDirty = false;

		if (ImGui::BeginTable("split", 2))
		{
			ImGui::TableNextColumn();
			ImGui::TableNextColumn();
			ImGui::Text("Ignore Overlap Block");

			for (int32 i = 0; i < 32; i++)
			{
				std::string& DisplayName = EngineTypes::Get()->CollisionChannelDisplayNames[i];
				// skip unused channels
				if (DisplayName == "")
				{
					continue;
				}

				uint8& Response = EnumArray[i];
				int32 Selected = Response;

				ImGui::TableNextColumn();
				ImGui::Text(DisplayName.c_str()); ImGui::SameLine();
				ImGui::TableNextColumn();
				ImGui::RadioButton(std::format("##Ignore{}", i).c_str(), &Selected, 0); ImGui::SameLine(55);
				ImGui::RadioButton(std::format("##Overlap{}", i).c_str(), &Selected, 1); ImGui::SameLine(110);
				ImGui::RadioButton(std::format("##Block{}", i).c_str(), &Selected, 2);

				if (Selected != Response)
				{
					Response = Selected;
					bDirty = true;
				}
			}

			ImGui::EndTable();
		}

		return bDirty;
	}

	bool DrawCollisionProfilePreset( const std::string& ProfileName )
	{
		CollisionResponseTemplate Preset;
		bool bFound = CollisionProfile::Get()->GetProfile(ProfileName, Preset);
		if (bFound)
		{
			ImGui::BeginDisabled();
			DrawCollisionEnabled("Collision Enabled", Preset.CollisionEnabled);
			DrawCollisionObjectType("Object Type", Preset.ObjectType);
			Preset.ResponseToChannels.Draw();
			ImGui::EndDisabled();
		}

		return false;
	}

	bool DrawCollisionProfileCustom( ECollisionEnabled& CollisionEnabled, ECollisionChannel& CollisionChannel, CollisionResponseContainer& ResponseToChannels )
	{
		bool bDirty = false;
		bDirty |= DrawCollisionEnabled("Collision Enabled", CollisionEnabled);
		bDirty |= DrawCollisionObjectType("Object Type", CollisionChannel);
		bDirty |= ResponseToChannels.Draw();

		return bDirty;
	}

#endif

// --------------------------------------------------------------------------------




        }  // namespace Drn