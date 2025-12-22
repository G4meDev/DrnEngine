#pragma once

template<typename Enum>
constexpr bool EnumHasAllFlags(Enum Flags, Enum Contains)
{
	return ( ( ( __underlying_type(Enum) )Flags ) & ( __underlying_type(Enum) )Contains ) == ( ( __underlying_type(Enum) )Contains );
}

template<typename Enum>
constexpr bool EnumHasAnyFlags(Enum Flags, Enum Contains)
{
	using UnderlyingType = __underlying_type(Enum);
	return ((UnderlyingType)Flags & (UnderlyingType)Contains) != 0;
}

template<typename Enum>
void EnumAddFlags(Enum& Flags, Enum FlagsToAdd)
{
	Flags |= FlagsToAdd;
}

template<typename Enum>
void EnumRemoveFlags(Enum& Flags, Enum FlagsToRemove)
{
	Flags &= ~FlagsToRemove;
}