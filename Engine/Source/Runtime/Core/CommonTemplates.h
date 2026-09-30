#pragma once

namespace Drn
{
	template <typename T>
	inline constexpr T Align(T Val, uint64 Alignment)
	{
		return (T)(((uint64)Val + Alignment - 1) & ~(Alignment - 1));
	}

	template <typename T>
	inline constexpr T AlignArbitrary(T Val, uint64 Alignment)
	{
		return (T)((((uint64)Val + Alignment - 1) / Alignment) * Alignment);
	}

	class Noncopyable
	{
	protected:
		Noncopyable() {}
		~Noncopyable() {}
	private:
		Noncopyable(const Noncopyable&);
		Noncopyable& operator=(const Noncopyable&);
	};
}