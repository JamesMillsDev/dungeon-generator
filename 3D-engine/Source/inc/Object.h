#pragma once

#include <bitset>

#include "Maths/Alias.h"

using std::hash;

class Object
{
public:
	virtual ~Object() = default;

public:
	[[nodiscard]] virtual uint64 GetHashCode() const = 0;

};

[[nodiscard]] inline uint64 HashCombine(uint64 seed, uint64 hash)
{
	return seed ^ (hash + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2));
}

template<typename T>
[[nodiscard]] static uint64 HashValue(T value)
{
	hash<T> hasher;

	return hasher(value);
}

template<typename... ARGS>
uint64 HashAll(ARGS&&... args)
{
	uint64 seed = 0;
	((seed = HashCombine(seed, HashValue(std::forward<ARGS>(args)))), ...);
	return seed;
}

namespace std
{
	template<>
	struct hash<Object>
	{
		uint64 operator()(const Object& obj) const noexcept
		{
			return obj.GetHashCode();
		}
	};
}
