#pragma once

#include "ForwardTypes.h"

#define REFLECTION_CAPTURE_SIZE 128
#define CUSTOM_COLLISION_PROFILE_NAME "Custom"

#define ECC_TO_BITFIELD( x ) ( 1 << ( x ) )
#define CRC_TO_BITFIELD( x ) ( 1 << ( x ) )

namespace Drn
{
	extern void RegisterGameTypes();

	enum class EActorType : uint16
	{
		StaticMeshActor = 0,
		CameraActor,
		PointLight,
		SpotLight,
		PostProcessVolume,
		DirectionalLight,
		SkyLight,
		Pawn,
		Controller,
		PlayerController,
		CameraManager,
		Character,
		DecalActor,
		SphereReflectionCapture,
		WheeledVehiclePawn,
		InstancedStaticMeshActor,
		ScenePointCloudImporter,
	};

	enum class EComponentType : uint32
	{
		Component = 0,
		SceneComponenet,
		StaticMeshComponent,
		CameraComponent,
		InputComponent,
		CharacterMovementComponent,
		SpringArmComponent,
		SphereReflectionCaptureComponent,
		WheeledVehicleMovementComponent,
		InstancedStaticMeshComponent,
	};

	enum class ELightType : uint8
	{
		PointLight,
		SpotLight,
		DirectionalLight,
		SkyLight,
	};

	enum class ERenderBufferCopySource
	{
		FinalColor,
		FinalColorPretonemap
	};

	class StaticShadowDepthMapData
	{
	public:
		//Matrix WorldToLight;
		int32 ShadowMapSizeX;
		int32 ShadowMapSizeY;
		std::vector<Float16> DepthSamples;

		StaticShadowDepthMapData() :
			//WorldToLight(Matrix::MatrixIdentity),
			ShadowMapSizeX(0),
			ShadowMapSizeY(0)
		{}

		void Empty();

		//size_t GetAllocatedSize() const
		//{
		//	return DepthSamples.GetAllocatedSize();
		//}
		friend Archive& operator<<(Archive& Ar, StaticShadowDepthMapData& ShadowMap);
		friend Archive& operator>>(Archive& Ar, StaticShadowDepthMapData& ShadowMap);
	};

	class ReflectionCaptureData
	{
	public:
		int32 CubemapSize;
		//float AverageBrightness;

		std::vector<uint8> SamplesData;

		ReflectionCaptureData()
			: CubemapSize(0)
		{}

		void Empty();

		friend Archive& operator<<(Archive& Ar, ReflectionCaptureData& CaptureData);
		friend Archive& operator>>(Archive& Ar, ReflectionCaptureData& CaptureData);
	};

	class Actor;
	class PrimitiveComponent;
	class BodyInstance;
	class World;

	struct HitResult
	{
		HitResult(const Vector& InTraceStart, const Vector& InTraceEnd, const Vector& InLocation,
			const Vector& InNormal, Actor* InHitActor, PrimitiveComponent* InHitComponent)
			: TraceStart(InTraceStart)
			, TraceEnd(InTraceEnd)
			, Location(InLocation)
			, Normal(InNormal)
			, HitActor(InHitActor)
			, HitComponent(InHitComponent)
		{
		}

		HitResult() : HitResult(Vector::ZeroVector, Vector::ZeroVector, Vector::ZeroVector,
			Vector::ZeroVector, nullptr, nullptr)
		{
		}

		Vector TraceStart;
		Vector TraceEnd;

		Vector Location;
		Vector Normal;

		Actor* HitActor;
		PrimitiveComponent* HitComponent;
	};

	struct RigidBodyContactInfo
	{
		RigidBodyContactInfo()
			: ContactPosition(Vector::ZeroVector)
			, ContactNormal(Vector::ZeroVector)
		{
		}

		RigidBodyContactInfo(const Vector& InContactPosition, const Vector& InContactNormal) 
			: ContactPosition(InContactPosition)
			, ContactNormal(InContactNormal)
		{
		}

		Vector ContactPosition;
		Vector ContactNormal;

		void SwapOrder();
	};

	struct CollisionImpactData
	{
	public:
		CollisionImpactData()
		{}

		std::vector<RigidBodyContactInfo> ContactInfos;
		void SwapContactOrders();
	};

	struct RigidBodyCollisionInfo
	{
		RigidBodyCollisionInfo()
			: m_Actor(nullptr)
			, m_Component(nullptr)
		{}
		
		Actor* m_Actor;
		PrimitiveComponent* m_Component;

		void SetFrom(const BodyInstance* BodyInst);
		BodyInstance* GetBodyInstance() const;
	};

	struct CollisionNotifyInfo
	{
		CollisionNotifyInfo() :
			bCallEvent0(true),
			bCallEvent1(true)
		{}

		bool bCallEvent0;
		bool bCallEvent1;

		RigidBodyCollisionInfo Info0;
		RigidBodyCollisionInfo Info1;

		CollisionImpactData RigidCollisionData;

		bool IsValidForNotify() const;
	};

// --------------------------------------------------------------------------------------------------------

	enum ECollisionChannel
	{
		ECC_WorldStatic,
		ECC_WorldDynamic,
		ECC_Pawn,
		ECC_Visibility,
		ECC_Camera,
		ECC_PhysicsBody,
		ECC_Vehicle,
		ECC_Destructible,

		ECC_EngineTraceChannel1,
		ECC_EngineTraceChannel2,
		ECC_EngineTraceChannel3,
		ECC_EngineTraceChannel4, 
		ECC_EngineTraceChannel5,
		ECC_EngineTraceChannel6,

		ECC_GameTraceChannel1,
		ECC_GameTraceChannel2,
		ECC_GameTraceChannel3,
		ECC_GameTraceChannel4,
		ECC_GameTraceChannel5,
		ECC_GameTraceChannel6,
		ECC_GameTraceChannel7,
		ECC_GameTraceChannel8,
		ECC_GameTraceChannel9,
		ECC_GameTraceChannel10,
		ECC_GameTraceChannel11,
		ECC_GameTraceChannel12,
		ECC_GameTraceChannel13,
		ECC_GameTraceChannel14,
		ECC_GameTraceChannel15,
		ECC_GameTraceChannel16,
		ECC_GameTraceChannel17,
		ECC_GameTraceChannel18,

		ECC_MAX,
	};

	enum ECollisionResponse
	{
		ECR_Ignore,
		ECR_Overlap,
		ECR_Block,
		ECR_MAX,
	};

#define NUM_ENGINE_COLLISION_CHANNELS 14
#define NUM_ENGINE_COLLISION_USED_CHANNELS 8

	struct CollisionResponseContainer
	{
		union
		{
			struct
			{
				//Reserved Engine Trace Channels
				uint8 WorldStatic;			// 0
				uint8 WorldDynamic;			// 1
				uint8 Pawn;					// 2
				uint8 Visibility;			// 3
				uint8 Camera;				// 4
				uint8 PhysicsBody;			// 5
				uint8 Vehicle;				// 6
				uint8 Destructible;			// 7

				// Unspecified Engine Trace Channels
				uint8 EngineTraceChannel1;   // 8
				uint8 EngineTraceChannel2;   // 9
				uint8 EngineTraceChannel3;   // 10
				uint8 EngineTraceChannel4;   // 11
				uint8 EngineTraceChannel5;   // 12
				uint8 EngineTraceChannel6;   // 13

				// Unspecified Game Trace Channels
				uint8 GameTraceChannel1;     // 14
				uint8 GameTraceChannel2;     // 15
				uint8 GameTraceChannel3;     // 16
				uint8 GameTraceChannel4;     // 17
				uint8 GameTraceChannel5;     // 18
				uint8 GameTraceChannel6;     // 19
				uint8 GameTraceChannel7;     // 20
				uint8 GameTraceChannel8;     // 21
				uint8 GameTraceChannel9;     // 22
				uint8 GameTraceChannel10;    // 23
				uint8 GameTraceChannel11;    // 24 
				uint8 GameTraceChannel12;    // 25
				uint8 GameTraceChannel13;    // 26
				uint8 GameTraceChannel14;    // 27
				uint8 GameTraceChannel15;    // 28
				uint8 GameTraceChannel16;    // 29 
				uint8 GameTraceChannel17;    // 30
				uint8 GameTraceChannel18;    // 31
			};
			uint8 EnumArray[32];
		};

		CollisionResponseContainer();
		CollisionResponseContainer(ECollisionResponse DefaultResponse);

		bool SetResponse(ECollisionChannel Channel, ECollisionResponse NewResponse);
		bool SetAllChannels(ECollisionResponse NewResponse);
		bool ReplaceChannels(ECollisionResponse OldResponse, ECollisionResponse NewResponse);
		inline ECollisionResponse GetResponse(ECollisionChannel Channel) const { return (ECollisionResponse)EnumArray[Channel]; }
		
		/** Set all channels from ChannelResponse Array **/
		//void UpdateResponsesFromArray(TArray<FResponseChannel> & ChannelResponses);
		//int32 FillArrayFromResponses(TArray<FResponseChannel> & ChannelResponses);
		
		static CollisionResponseContainer CreateMinContainer(const CollisionResponseContainer& A, const CollisionResponseContainer& B);
		static const struct CollisionResponseContainer& GetDefaultResponseContainer() { return DefaultResponseContainer; }
		
		bool operator==(const CollisionResponseContainer& Other) const
		{
			return std::memcmp(EnumArray, Other.EnumArray, sizeof(Other.EnumArray)) == 0;
		}
		bool operator!=(const CollisionResponseContainer& Other) const
		{
			return std::memcmp(EnumArray, Other.EnumArray, sizeof(Other.EnumArray)) != 0;
		}

		friend Archive& operator<<(Archive& Ar, CollisionResponseContainer& Container);
		friend Archive& operator>>(Archive& Ar, CollisionResponseContainer& Container);

#if WITH_EDITOR
		bool Draw();
#endif

	private:

		static CollisionResponseContainer DefaultResponseContainer;

		//friend class UCollisionProfile;
	};

	enum class ECollisionEnabled : uint8 
	{ 
		NoCollision,
		QueryOnly,
		PhysicsOnly,
		QueryAndPhysics
	};

	inline bool CollisionEnabledHasPhysics(ECollisionEnabled CollisionEnabled)
	{
		return (CollisionEnabled == ECollisionEnabled::PhysicsOnly) ||
				(CollisionEnabled == ECollisionEnabled::QueryAndPhysics);
	}

	inline bool CollisionEnabledHasQuery(ECollisionEnabled CollisionEnabled)
	{
		return (CollisionEnabled == ECollisionEnabled::QueryOnly) ||
				(CollisionEnabled == ECollisionEnabled::QueryAndPhysics);
	}

	struct CollisionResponseTemplate
	{
		std::string Name;
		ECollisionEnabled CollisionEnabled;
		ECollisionChannel ObjectType;
		CollisionResponseContainer ResponseToChannels;

		CollisionResponseTemplate();
		CollisionResponseTemplate(const std::string&& InName, ECollisionEnabled InCollisionEnabled, ECollisionChannel InObjectType);
	};

	struct CollisionProfile
	{
		std::unordered_map<std::string, CollisionResponseTemplate> Profiles;
		std::vector<std::string> ProfileNames;

		static CollisionProfile* Get();
		bool GetProfile(const std::string& ProfileName, CollisionResponseTemplate& ProfileData) const;
		void AddProfile(const CollisionResponseTemplate& ProfileData);
		bool UpdateProfileChannelResponse(const std::string& ProfileName, ECollisionChannel CollisionChannel, ECollisionResponse CollisionResponse);

		static CollisionProfile* SingletonInstance;
	};

	enum EPhysXFilterDataFlags
	{
		EPDF_SimpleCollision	=	0x0001,
		EPDF_ComplexCollision	=	0x0002,
		EPDF_CCD				=	0x0004,
		EPDF_ContactNotify		=	0x0008,
		EPDF_StaticShape		=	0x0010,
		EPDF_ModifyContacts		=   0x0020,
		EPDF_KinematicKinematicPairs = 0x0040,
	};

	struct PhysicsFilterBuilder
	{
		PhysicsFilterBuilder(ECollisionChannel InObjectType, uint8 MaskFilter, const CollisionResponseContainer& ResponseToChannels);

		inline void ConditionalSetFlags(EPhysXFilterDataFlags Flag, bool bEnabled)
		{
			if (bEnabled)
			{
				Word3 |= Flag;
			}
		}

		inline void GetQueryData(uint32 ActorID, uint32& OutWord0, uint32& OutWord1, uint32& OutWord2, uint32& OutWord3) const
		{
			OutWord0 = ActorID;
			OutWord1 = BlockingBits;
			OutWord2 = TouchingBits;
			OutWord3 = Word3;
		}

		inline void GetSimData(uint32 BodyIndex, uint32 ComponentID, uint32& OutWord0, uint32& OutWord1, uint32& OutWord2, uint32& OutWord3) const
		{
			OutWord0 = BodyIndex;
			OutWord1 = BlockingBits;
			OutWord2 = ComponentID;
			OutWord3 = Word3;
		}

		inline void GetCombinedData(uint32& OutBlockingBits, uint32& OutTouchingBits, uint32& OutObjectTypeAndFlags) const
		{
			OutBlockingBits = BlockingBits;
			OutTouchingBits = TouchingBits;
			OutObjectTypeAndFlags = Word3;
		}

	private:
		uint32 BlockingBits;
		uint32 TouchingBits;
		uint32 Word3;
	};

	inline uint32 CreateChannelAndFilter(ECollisionChannel CollisionChannel, uint8 MaskFilter)
	{
		uint32 ResultMask = (uint32(MaskFilter) << 5) | (uint32)CollisionChannel;
		return ResultMask << 21;
	}

	inline ECollisionChannel GetCollisionChannel(uint32 Word3)
	{
		uint32 ChannelMask = (Word3 << 6) >> (32 - 5);
		return (ECollisionChannel)ChannelMask;
	}

	struct CollisionFilterData
	{
		uint32 Word0;
		uint32 Word1;
		uint32 Word2;
		uint32 Word3;

		inline CollisionFilterData()
			: Word0(0)
			, Word1(0)
			, Word2(0)
			, Word3(0)
		{}
	};

	void CreateShapeFilterData( const uint8 MyChannel, const uint8 MaskFilter, const int32 ActorID, const CollisionResponseContainer& ResponseToChannels,
		uint32 ComponentID, uint16 BodyIndex, CollisionFilterData& OutQueryData, CollisionFilterData& OutSimData,
		bool bEnableCCD, bool bEnableContactNotify, bool bStaticShape, bool bModifyContacts = false);

#if WITH_EDITOR

	bool DrawCollisionProfile(const std::string& Label, std::string& ProfileName);
	bool DrawCollisionObjectType(const std::string& Label, ECollisionChannel& CollisionChannel);
	bool DrawCollisionEnabled(const std::string& Label, ECollisionEnabled& CollisionEnabled);
	bool DrawCollisionProfilePreset(const std::string& ProfileName);
	bool DrawCollisionProfileCustom(ECollisionEnabled& CollisionEnabled, ECollisionChannel& CollisionChannel, CollisionResponseContainer& ResponseToChannels);

#endif

// --------------------------------------------------------------------------------------------------------
	
	enum class EBufferVisualization
	{
		FinalImage,
		BaseColor,
		Metallic,
		Roughness,
		WorldNormal,
		MaterialAO,
		ScreenSpaceAO,
		CombinedAO,
		Depth,
		SubsurfaceColor,
		ShadingModel,
		Velocity,
		PreTonemapColor,
		LinearDepth,
		Bloom,
		ScreenSpaceReflection
	};

	struct NonCopyable
	{
		NonCopyable& operator=(const NonCopyable&) = delete;
		NonCopyable(const NonCopyable&) = delete;
		NonCopyable() = default;
	};

	class EngineTypes
	{
	public:
	
		EngineTypes() {};
		~EngineTypes() {};

		void RegisterSerializableActor(EActorType ActorType, std::function< Actor* (World* InWorld, Archive& Ar)>&& Func);

		void Register();

		static EngineTypes* Get();
		static EngineTypes* m_SingletonInstance;
		std::unordered_map<EActorType, std::function<Actor*(World* InWorld, Archive& Ar)>> m_ActorSerializationMap;

		std::string CollisionChannelDisplayNames[32];
	};

#define REGISTER_SERIALIZABLE_ACTOR( type , class )																		\
	EngineTypes::Get()->RegisterSerializableActor( static_cast<EActorType>( type ), []( World * InWorld, Archive& Ar )	\
	{ class* NewActor = InWorld->SpawnActor<class>(); NewActor->Serialize(Ar); return NewActor; });
}