#pragma once

#define ENGINE_MAX_ACTOR_TYPES 32768
#define ENGINE_MAX_COMPONENT_TYPES INT32_MAX

enum class EGameActorType : uint16
{
    TestPlayerCharacter = ENGINE_MAX_ACTOR_TYPES,
    RaceVehiclePawn,
};

enum class EGameComponentType : uint32
{
    RaceVehicleMovementComponent = ENGINE_MAX_COMPONENT_TYPES,
};