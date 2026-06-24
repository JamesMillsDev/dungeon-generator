#pragma once

#include "Maths/Alias.h"

#include <functional>

using CleanupFunction = std::function<void()>;

class ResourceStack
{
private:
	CleanupFunction* m_stack;
	int32 m_top;
	int32 m_capacity;
	int32 m_growSize;

public:
	explicit ResourceStack(int32);
	~ResourceStack();

public:
	void Push(const CleanupFunction&);

};