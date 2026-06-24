#include "pch.h"
#include "ResourceStack.h"

ResourceStack::ResourceStack(const int32 growSize)
	: m_stack{ new CleanupFunction[growSize] }, m_top{ -1 }, m_capacity{ growSize }, m_growSize{ growSize }
{

}

ResourceStack::~ResourceStack()
{
	while (m_top > -1)
	{
		m_stack[m_top--]();
	}

	delete[] m_stack;
	m_stack = nullptr;
}

void ResourceStack::Push(const CleanupFunction& cleanupFunction)
{
	const int newTop = m_top + 1;
	if (newTop >= m_capacity)
	{
		m_stack = static_cast<CleanupFunction*>(realloc(m_stack, m_capacity + m_growSize));
		m_capacity += m_growSize;
	}

	m_stack[newTop] = cleanupFunction;
	m_top++;
}